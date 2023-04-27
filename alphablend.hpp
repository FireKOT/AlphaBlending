#ifndef _CRACK_HPP_
#define _CRACK_HPP_


#include <SFML/Graphics.hpp>
#include <immintrin.h>


struct blend {

    int width, height;
    int size;

    sf::Uint8 *front_algn_pxls, *back_algn_pxls;
    sf::Uint8 *blend_algn_pxls;
    sf::Image  blend_img;
};


void checkWindowIvents (sf::RenderWindow *window);

void loadImage  (blend *data, const char *front_img_name, const char *back_img_name);
void alphaBlend (blend *data);
void procPixels (const sf::Uint8 *front_img, const sf::Uint8 *back_img, sf::Uint8 *out_img, int offset);
void makeImage  (blend *data);
void blendDtor  (blend *data);


#endif