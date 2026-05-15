# Real Car ADAS Monitor Architecture

## Project Structure

real-car-adas-monitor/
├── CMakeLists.txt ← главный файл сборки
├── README.md ← описание проекта
├── .gitignore ← что не загружать в Git
│
├── src/ ← весь исходный код
│ ├── main.cpp
│ ├── obd_parser.h ← парсер OBD CSV данных
│ ├── obd_parser.cpp
│ ├── onnx_classifier.h ← ИИ-классификатор стиля вождения
│ ├── onnx_classifier.cpp
│ ├── dashboard.h ← приборная панель (OpenCV)
│ ├── dashboard.cpp
│ ├── dms_monitor.h ← детекция состояния водителя
│ ├── dms_monitor.cpp
│ ├── dms_hud.h ← HUD для камеры
│ └── dms_hud.cpp
│
├── tests/
│ └── test_obd_parser.cpp ← unit-тесты
│
├── models/ ← ONNX модели (не загружаются в Git)
├── data/ ← CSV файлы (не загружаются в Git)
├── output/ ← результаты (скриншоты, видео)
└── docs/
 └── architecture.md ← описание архитектуры
 
## System Flow

CSV Data
    ↓
OBD Parser
    ↓
ONNX Classifier
    ↓
Dashboard / HUD
    ↓
Driver Alerts
