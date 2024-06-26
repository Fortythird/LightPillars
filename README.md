# Разработка алгоритма отрисовки световых столбов в реальном времени на основе соответствующих физических процессов

Данная работа выполнена с целью физически достоверного воспроизведения такого атмосферного оптического явления, как световые столбы. Результаты представлены в качестве ВКР по направлению "Технологии разработки компьютерных игр" Университета ИТМО (Санкт-Петербург) в 2024 году.

![Pillars in real life](Media/pillars.jpg)

## Запуск проекта

Для запуска проекта потребуется среда с поддержкой C++ и DirectX 11. Рекомендуется установить Microsoft Visuial Studio 2022, при этом следует добавить пакет "Разработка на языке C++". Все остальные ресурсы (модели, текстуры, шейдеры) загрузятся вместе с проектом.

![MVS package](Media/VisualStudio.png)

Далее, после запуска Visual Studio необходимо выбрать опцию "Открыть проект или решение" и выбрать директорию, куда были загружены репозитория.

![Open project](Media/OpenProject.png)

После того, как проект загрузился, его можно будет запустить через соответствующую кнопку в верхней панели.

![Run project](Media/RunProject.png)

## Обзор решения

Цель демонстрации: наглядно показать взаимодействие моделируемого светового столба с окружением.

1. Структура столба зависит от позиции наблюдателя. В расчет включены атмосферное поглощение, потери при отражении, ширина столба, теоретическое размещение кристаллов льда

![Demo 1](Media/Demo1.png) ![Demo 2](Media/Demo2.png) ![Demo 3](Media/Demo3.png)

2. Учитывается глубина размещения столба на сцене

![Demo 4](Media/Demo4.png)

3. На структуру столба влияют объекты, находящиеся в объеме формирования явления

![Demo 5](Media/Demo5.png)

4. Поддерживается отрисовка множества столбов

![Demo 6](Media/Demo6.png)