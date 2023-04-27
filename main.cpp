#include <stdio.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <immintrin.h>

#include "alphablend.hpp"


int main () {

    int s_width = 1000, s_height = 1000;

    sf::RenderWindow window(sf::VideoMode(s_width, s_height), "Mandelbrote");
    window.setFramerateLimit(60);

    sf::Texture texture = {};
    if (!texture.create(s_width * 4, s_height * 4)) {

        printf("Faild texture creation!\n");
        return 1;
    }
    sf::Sprite sprite(texture);

    blend data = {};

    loadImage(&data, "catgirl.png", "cat.jpg");

    int repeats = 5000;
    sf::Clock clock = {};
    while (window.isOpen()) {

        checkWindowIvents(&window);

        clock.restart();

        for (int i = 0; i < repeats; i++) {

            alphaBlend(&data);
        }
        
        long long elapsed = clock.restart().asMicroseconds();
        printf("mcs: %lld fps: %lld\n", elapsed / repeats, 1000000 / (elapsed / repeats));

        makeImage(&data);

        texture.update(data.blend_img);

        window.clear(sf::Color::Black);

        window.draw(sprite);

        window.display();
    }

    blendDtor(&data);

    return 0;
}