# Альфа Блендинг

## Цель

Оптимизация с помощью SIMD инструкций

## Программа

Программа накладывает друг на друга картинки одинакого размера, кратного восьми, с учетом прозрачности по формуле: 

$$\frac{front_{pixel} * front_{alpha} + back_{pixel} * (255 - front_{alpha})}{256}$$

и выводит результат на экран в отдельном окне.

## Теория

SIMD инструкции позволяют оптимизировать исполнение большого количества однотипных операций, данном случаее блендинга пикселей накладывающихся друг на друга картинок. 

## Оптимизация

Ускорения удалось достичь с помощью инструкций AVX2, благодоря которым за одну итерацию цикла удается обрабатывать 8 пикселей вместо одного.

Рассмотрим фрагменты с обсчетом пикселей оптимизированного и не оптимизированного кода и ассемблерный код эти фрагментов, получаемый при компиляции с флагом -O2 (получен с помощью сервиса [GodBolt](https://godbolt.org/))

Код без оптимизации:
```
data->blend_algn_pxls[offset]     = (unsigned char) ((data->front_algn_pxls[offset]     * fAlpha + data->back_algn_pxls[offset]     * bAlpha) >> 8);
data->blend_algn_pxls[offset + 1] = (unsigned char) ((data->front_algn_pxls[offset + 1] * fAlpha + data->back_algn_pxls[offset + 1] * bAlpha) >> 8);
data->blend_algn_pxls[offset + 2] = (unsigned char) ((data->front_algn_pxls[offset + 2] * fAlpha + data->back_algn_pxls[offset + 2] * bAlpha) >> 8);
```

и часть его ассемблерного представления:
```
mov     rsi, QWORD PTR [rdx+16]
mov     r10, QWORD PTR [rdx+32]
movzx   ebx, BYTE PTR [rsi+1+rax]
mov     rsi, QWORD PTR [rdx+24]
movzx   esi, BYTE PTR [rsi+1+rax]
imul    ebx, r9d
imul    esi, ecx
add     ebx, esi
movzx   ebx, bh
mov     BYTE PTR [r10+1+rax], bl
```

Фрагмент оптимизированного варианта:
```
unsigned char set_zero = 255;
__m256i max_alpha = _mm256_set1_epi16(255);

//load cur pixels
__m256i front = _mm256_load_si256((__m256i const*) (front_img + offset));

//load lover & hight halves of reg 
__m256i front_l = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(front, 0));
__m256i front_h = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(front, 1));

__m256i alpha_l = _mm256_shuffle_epi8(front_l, mask);
__m256i alpha_h = _mm256_shuffle_epi8(front_h, mask);

//front * alpha
front_l = _mm256_mullo_epi16(front_l, alpha_l);
front_h = _mm256_mullo_epi16(front_h, alpha_h);
```

и его ассемблерное представление:
```
vpmovzxbw       ymm4, xmm2
vpmovzxbw       ymm6, xmm0

vextracti64x2   xmm2, ymm2, 0x1
vextracti64x2   xmm0, ymm0, 0x1

vpmovzxbw       ymm2, xmm2
vpmovzxbw       ymm3, xmm0
vpbroadcastw    ymm0, eax

vpshufb ymm5, ymm2, ymm5
vpmullw ymm4, ymm4, ymm7

vpmullw ymm1, ymm1, ymm6
```

Из этого фрагмента видно, что, в отличие от наивной реализации, при использовании AVX2 инструкций используются xmm регистры, позволяющие обрабатывать восемь пиксилей за итерацию.

## Результаты измерений

Измерения производились на ноутбуке, подключенном к сети, при температуре внешней среды 298К и флаге оптимизации -O2 на изображениях размером 1000 на 1000 пикселей

Версия  | FPS
--------|----
Наивная | 320
С AVX2  | 820

## Выводы

Использование SIMD (в частности AVX2) технологий позволяет ускорить блендинг изображения в 4 раза
