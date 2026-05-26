# Real Car ADAS Monitor

Система мониторинга автомобиля на C++: в одном окне отображает телеметрию OBD-II и анализирует состояние водителя с веб-камеры.

## Что делает проект

Приложение объединяет несколько модулей:

читает CSV с телеметрией автомобиля;
- классифицирует стиль вождения через ONNX Runtime;
- рисует приборную панель OpenCV;
- анализирует лицо, глаза и направление головы водителя;
- объединяет всё в одном окне в реальном времени;
- записывает результат работы в видеофайл и журнал алертов.



## Технологии

- C++17 =	основной язык проекта
- CMake = 	сборка проекта
- OpenCV 4.x = 	видео, камера, отрисовка HUD и DNN
- ONNX Runtime =	inference модели классификации
- Google Test = 	unit-тесты
- Python 3 =	подготовка датасета и обучение модели



## Структура проекта

real-car-adas-monitor/
├── CMakeLists.txt
├── README.md
├── Doxyfile
├── src/
│   ├── main.cpp
│   ├── obd_parser.h / obd_parser.cpp
│   ├── onnx_classifier.h / onnx_classifier.cpp
│   ├── dashboard.h / dashboard.cpp
│   ├── dms_monitor.h / dms_monitor.cpp
│   ├── dms_hud.h / dms_hud.cpp
│   └── shared_state.h
├── tests/
├── models/
├── data/
├── output/
└── docs/



## Сборка

Папка `build` должна быть отдельной от исходников.
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```


## Запуск
Из папки `build`
./RealCarMonitor.exe



## Что должно лежать в папках

data/
- obd_data.csv
models/
- driver_classifier.onnx
- normalization_params.json
- deploy.prototxt
- res10_300x300_ssd_iter_140000.caffemodel
- haarcascade_eye.xml



## Результаты

Во время запуска приложение:

- показывает окно 1280×480;
- слева отображает dashboard;
- справа отображает DMS HUD;
- пишет видео в output/result_situation2.mp4;
- пишет журнал событий в output/dms_alerts.log


## Формат output/dms_alerts.log

2026-05-27 14:03:11 | drowsy=1 distracted=0 aggressive=0



## Тесты

Запуск unit-тестов:

```
cmake --build .
./RunTests.exe
```



## Ключевые возможности

Добавлены комментарии вида /** ... */ для:

- каждого класса;
- каждого публичного метода;




## Ключевые возможности

- многопоточная обработка OBD и видео;
- визуализация телеметрии в реальном времени;
- детекция лица, глаз и направления головы;
- логирование алертов;
- запись финального видео.
