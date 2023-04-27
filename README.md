# Альфа Блендинг

## Цель

Оптимизация с помощью SIMD инструкций

## Программа

Программа накладывает друг на друга картинки одинакого размера, кратного восьми, с учетом прозрачности по формуле: 

$$front_{pixel} * front_{alpha} + back_{pixel} * (255 - front_{alpha})$$

и выводит результат на экран в отдельном окне.

## Теория

SIMD инструкции позволяют оптимизировать исполнение большого количества однотипных операций, данном случаее блендинга пикселей накладывающихся друг на друга картинок. 

## Оптимизация

Ускорения удалось достичь с помощью инструкций AVX2, благодоря которым за одну итерацию цикла удается обрабатывать 8 пикселей вместо одного.

Рассмотрим фрагменты с обсчетом пикселей оптимизированного и не оптимизированного кода, скомпилированные с флагом -O2

```
int offset = (y * data->width + x) * 4;
sf::Uint8 fAlpha = data->front_algn_pxls[offset + 3], bAlpha = (255 - data->front_algn_pxls[offset + 3]);

data->blend_algn_pxls[offset]     = (sf::Uint8) ((data->front_algn_pxls[offset]     * fAlpha + data->back_algn_pxls[offset]     * bAlpha) >> 8);
data->blend_algn_pxls[offset + 1] = (sf::Uint8) ((data->front_algn_pxls[offset + 1] * fAlpha + data->back_algn_pxls[offset + 1] * bAlpha) >> 8);
data->blend_algn_pxls[offset + 2] = (sf::Uint8) ((data->front_algn_pxls[offset + 2] * fAlpha + data->back_algn_pxls[offset + 2] * bAlpha) >> 8);
data->blend_algn_pxls[offset + 3] = 255;
```
