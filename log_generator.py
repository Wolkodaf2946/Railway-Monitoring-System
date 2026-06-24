import os
import random
import csv
from datetime import datetime, timedelta

# --- КОНФИГУРАЦИЯ ---
ROOT_DIR = "data_logs"
TRAINS_COUNT = 5          # Количество поездов
WAGONS_PER_TRAIN = 60     # Вагонов в составе
DURATION_HOURS = 2        # Длительность поездки (в часах)
LOG_INTERVAL_MIN = 1      # Запись каждую минуту

# Вероятности дефектов (в сумме < 1.0, остальное - здоровые)
CHANCE_BRAKE_FAILURE = 0.05  # Отказ торможения
CHANCE_SELF_BRAKING = 0.05   # Самосраб (тормозит сам)
CHANCE_STUCK_BRAKE = 0.05    # Неотпуск (едет на тормозах)
CHANCE_OVERPRESSURE = 0.05   # Передавление (> 4.5)
CHANCE_ROD_ISSUE = 0.05      # Проблема штока (L)
CHANCE_OVERHEAT = 0.05       # Перегрев (T)

def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

def get_wagon_diagnosis():
    """Случайно определяет, чем болен вагон на всю поездку"""
    r = random.random()
    if r < CHANCE_BRAKE_FAILURE: return 'brake_failure'
    r -= CHANCE_BRAKE_FAILURE
    if r < CHANCE_SELF_BRAKING: return 'self_braking'
    r -= CHANCE_SELF_BRAKING
    if r < CHANCE_STUCK_BRAKE: return 'stuck_brake'
    r -= CHANCE_STUCK_BRAKE
    if r < CHANCE_OVERPRESSURE: return 'overpressure'
    r -= CHANCE_OVERPRESSURE
    if r < CHANCE_ROD_ISSUE: return 'rod_issue'
    r -= CHANCE_ROD_ISSUE
    if r < CHANCE_OVERHEAT: return 'overheat'
    
    return 'healthy'

def simulate_step(is_braking_mode, diagnosis, ambient_temp):
    """
    Генерирует одну строку данных на основе режима поезда и диагноза вагона.
    is_braking_mode: True, если машинист нажал тормоз (снизил давление в ТМ).
    """
    
    # 1. БАЗОВАЯ ФИЗИКА (ИДЕАЛЬНЫЙ ВАГОН)
    if is_braking_mode:
        # Торможение: Магистраль падает, Цилиндр растет
        p_pipe = random.uniform(3.9, 4.2)  # Разрядка
        p_cyl = random.uniform(3.8, 4.2)   # Наполнение (груженый режим)
        rod = random.randint(80, 115)      # Нормальный выход
    else:
        # Поездное положение: Магистраль полная, Цилиндр пуст
        p_pipe = random.uniform(5.0, 5.2)  # Зарядка
        p_cyl = 0.0
        rod = 0

    # 2. ПРИМЕНЕНИЕ ДИАГНОЗОВ (ИСКАЖЕНИЕ ФИЗИКИ)
    
    if diagnosis == 'brake_failure':
        # Отказ: Магистраль реагирует (падает), но Цилиндр пуст
        # Шток тоже не выходит
        p_cyl = 0.0
        rod = 0 

    elif diagnosis == 'self_braking':
        # Самосраб: В режиме "Поездное" (не тормозим) цилиндр растет
        if not is_braking_mode:
            p_cyl = random.uniform(0.8, 1.5) # Чуть подтормаживает
            rod = random.randint(20, 50)     # Шток чуть вышел
    
    elif diagnosis == 'stuck_brake':
        # Неотпуск: В режиме "Поездное" (уже отпустили) цилиндр держит давление
        if not is_braking_mode:
            p_cyl = random.uniform(3.0, 3.5) # Держит сильно
            rod = random.randint(80, 110)    # Шток не ушел
            
    elif diagnosis == 'overpressure':
        # Передавление: При торможении давление улетает за 4.5
        if is_braking_mode:
            p_cyl = random.uniform(4.6, 5.0)

    elif diagnosis == 'rod_issue':
        # Проблема штока: Если тормозим, выход не в норме
        if is_braking_mode or p_cyl > 0.5:
            # Либо слишком мало, либо слишком много
            rod = random.choice([random.randint(20, 60), random.randint(127, 131)])

    # 3. ТЕМПЕРАТУРА
    # База = улица + 20. Торможение греет.
    base_t = ambient_temp + 20
    if p_cyl > 0.5: # Если колодки прижаты
        base_t += random.uniform(15, 30)
    
    # Если вагон болен перегревом или едет на неотпуске - греется сильнее
    if diagnosis == 'overheat' or diagnosis == 'stuck_brake' or (diagnosis == 'self_braking' and not is_braking_mode):
        base_t += random.uniform(40, 60) # Будет > 80

    # Генерируем 4 колеса с шумом
    temps = []
    for _ in range(4):
        t = base_t + random.uniform(-2, 2)
        temps.append(int(t))

    return round(p_pipe, 2), round(p_cyl, 2), int(rod), temps

def generate_logs():
    print(f"Генерация 'умных' логов в {ROOT_DIR}...")
    ensure_dir(ROOT_DIR)
    
    start_time = datetime.now() - timedelta(hours=DURATION_HOURS)

    for t_idx in range(TRAINS_COUNT):
        # Номер поезда (чет/нечет)
        train_num = random.randint(1000, 9998)
        if t_idx % 2 == 0: train_num = train_num if train_num % 2 == 0 else train_num + 1
        else: train_num = train_num if train_num % 2 != 0 else train_num + 1
            
        train_dir = os.path.join(ROOT_DIR, f"train_{train_num}")
        ensure_dir(train_dir)
        print(f"Поезд {train_num}...")

        for _ in range(WAGONS_PER_TRAIN):
            wagon_id = str(random.randint(10000000, 99999999))
            file_path = os.path.join(train_dir, f"{wagon_id}.log")
            
            # Ставим диагноз вагону
            diagnosis = get_wagon_diagnosis()
            ambient = random.randint(5, 15)

            with open(file_path, 'w', newline='', encoding='utf-8') as f:
                writer = csv.writer(f)
                writer.writerow(["Time", "P_Pipe", "P_Cyl", "Rod", "T1", "T2", "T3", "T4", "Ambient"])
                
                curr_time = start_time
                total_minutes = DURATION_HOURS * 60
                
                # Сценарий: Едем 20 мин, Тормозим 5 мин (цикл)
                for m in range(total_minutes):
                    cycle_pos = m % 25
                    # Тормозим последние 5 минут цикла
                    is_braking = cycle_pos >= 20 
                    
                    p_pipe, p_cyl, rod, temps = simulate_step(is_braking, diagnosis, ambient)
                    
                    row = [
                        curr_time.strftime("%Y-%m-%d %H:%M:%S"),
                        p_pipe, p_cyl, rod, 
                        temps[0], temps[1], temps[2], temps[3],
                        ambient
                    ]
                    writer.writerow(row)
                    curr_time += timedelta(minutes=LOG_INTERVAL_MIN)

    print("Генерация завершена. Перезапустите приложение.")

if __name__ == "__main__":
    generate_logs()