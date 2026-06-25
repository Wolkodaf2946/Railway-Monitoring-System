# Railway Monitoring System

Скрипт для генерации логов:
- `log_generator.py`

Основная C++-реализация находится в `src/` и собирается через CMake.

## Сборка

```bash
cmake -S . -B build
cmake --build build
```

## Запуск

```bash
./build/railway_monitor
```

Приложение ожидает папку `data_logs/` в корне проекта и читает те же `.log`/CSV-файлы.

## Генерация логов

```bash
python3 log_generator.py
```
