#include <immintrin.h>
#include <stdio.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "alphablend.hpp"
#include "config.hpp"


static const int mmSIZE = 8;


void checkWindowIvents (sf::RenderWindow *window) {

    RET_ON_VAL(!window, ERR_INV_ARG, );

    sf::Event event = {};
    while (window->pollEvent(event))
    {
        if (event.type == sf::Event::Closed ||\
            sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {

            window->close();
        }
    }
}

void loadImage  (blend *data, const char *front_img_name, const char *back_img_name) {

    RET_ON_VAL(!data || !front_img_name || !back_img_name, ERR_INV_ARG, );

    sf::Image front = {}, back = {};
    if (!front.loadFromFile(front_img_name)) {

        printf("Failed to load front image\n");
        return;
    }
    if (!back.loadFromFile(back_img_name)) {

        printf("Failed to load back image\n");
        return;
    }
    if (front.getSize().x != back.getSize().x || front.getSize().y != back.getSize().y) {

        printf ("Error! Diffrent images sizes!\n");
        return;
    }

    data->width = back.getSize().x, data->height = back.getSize().y;
    data->size = data->width * data->height * 4;

    const sf::Uint8 *front_pxls = front.getPixelsPtr();
    const sf::Uint8  *back_pxls =  back.getPixelsPtr();

    data->front_algn_pxls = (sf::Uint8*) aligned_alloc(32, data->size * sizeof(sf::Uint8));
     data->back_algn_pxls = (sf::Uint8*) aligned_alloc(32, data->size * sizeof(sf::Uint8));
    data->blend_algn_pxls = (sf::Uint8*) aligned_alloc(32, data->size * sizeof(sf::Uint8));

    if (!data->front_algn_pxls || !data->back_algn_pxls) {

        printf("Error! Faild to allocate alligned space for images\n");
        return;
    }

    for (int i = 0; i < data->size; i++) {

        data->front_algn_pxls[i] = front_pxls[i];
         data->back_algn_pxls[i] =  back_pxls[i];
    }
}

void alphaBlend (blend *data) {

    RET_ON_VAL(!data || data->width < 0 || data->height < 0, ERR_INV_ARG, );
    RET_ON_VAL((long long) data->front_algn_pxls % 32 || (long long) data->back_algn_pxls % 32 || (long long) data->blend_algn_pxls % 32, ERR_INV_ARG, );

    for (int y = 0; y < data->height; y++) {

        for (int x = 0; x < data->width; x += mmSIZE) {

            int offset = (y * data->width + x) * 4;
            procPixels(data->front_algn_pxls, data->back_algn_pxls, data->blend_algn_pxls, offset);
        }
    }
}

void procPixels (const sf::Uint8 *front_img, const sf::Uint8 *back_img, sf::Uint8 *out_img, int offset) {

    RET_ON_VAL(front_img == 0 || back_img == 0 || out_img == 0 || offset < 0 || offset % 32, ERR_INV_ARG, );

    unsigned char set_zero = 255;
    __m256i max_alpha = _mm256_set1_epi16(255);

    //load cur pixels
    __m256i front = _mm256_load_si256((__m256i const*) (front_img + offset));
    __m256i  back = _mm256_load_si256((__m256i const*)  (back_img + offset));

    //load lover & hight halves of reg 
    __m256i front_l = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(front, 0));
    __m256i front_h = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(front, 1));

    __m256i back_l = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(back, 0));
    __m256i back_h = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(back, 1));

    //get front alpha channel
    __m256i mask = _mm256_set_epi8(set_zero, 30, set_zero, 30, set_zero, 30, set_zero, 30,
                                   set_zero, 22, set_zero, 22, set_zero, 22, set_zero, 22,
                                   set_zero, 14, set_zero, 14, set_zero, 14, set_zero, 14,
                                   set_zero,  6, set_zero,  6, set_zero,  6, set_zero,  6);

    __m256i alpha_l = _mm256_shuffle_epi8(front_l, mask);
    __m256i alpha_h = _mm256_shuffle_epi8(front_h, mask);

    //front * alpha
    front_l = _mm256_mullo_epi16(front_l, alpha_l);
    front_h = _mm256_mullo_epi16(front_h, alpha_h);

    //back * (max_alpha - front_alpha)
    back_l = _mm256_mullo_epi16(back_l, _mm256_sub_epi16(max_alpha, alpha_l));
    back_h = _mm256_mullo_epi16(back_h, _mm256_sub_epi16(max_alpha, alpha_h));

    //front + back
    __m256i ans_l = _mm256_add_epi16(front_l, back_l);
    __m256i ans_h = _mm256_add_epi16(front_h, back_h);

    //set significant bytes in the highest & the lovest positions
    __m256i merge_mask = _mm256_set_epi8(31,       29,       27,       25,       23,       21,       19,       17,
                                         set_zero, set_zero, set_zero, set_zero, set_zero, set_zero, set_zero, set_zero,
                                         set_zero, set_zero, set_zero, set_zero, set_zero, set_zero, set_zero, set_zero,
                                         15,       13,       11,       9,        7,        5,        3,        1       );

    ans_l = _mm256_shuffle_epi8(ans_l, merge_mask);
    ans_h = _mm256_shuffle_epi8(ans_h, merge_mask);

    __m256i colors = _mm256_set_m128i(_mm_add_epi8(_mm256_extracti128_si256(ans_h, 0), _mm256_extracti128_si256(ans_h, 1)),
                                      _mm_add_epi8(_mm256_extracti128_si256(ans_l, 0), _mm256_extracti128_si256(ans_l, 1)));

    _mm256_store_si256((__m256i*)(out_img + offset), colors);
} 

void makeImage (blend *data) {

    RET_ON_VAL(!data || !data->blend_algn_pxls, ERR_INV_ARG, );

    data->blend_img.create(data->width, data->height, data->blend_algn_pxls);
}

void blendDtor (blend *data) {

    RET_ON_VAL(!data, ERR_INV_ARG, );

    free(data->front_algn_pxls);
    free(data-> back_algn_pxls);
    free(data->blend_algn_pxls);

    data->width  = 0;
    data->height = 0;
    data->size   = 0;

    data->front_algn_pxls = nullptr;
    data-> back_algn_pxls = nullptr;
    data->blend_algn_pxls = nullptr;
}
