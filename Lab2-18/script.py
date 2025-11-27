#!/usr/bin/env python3
import matplotlib
matplotlib.use("Qt5Agg")


import subprocess
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os


def run_experiment(text, pattern, threads, repetitions=3):
    """Запускает программу с заданными параметрами через стандартный ввод"""
    times = []
    matches = 0
    
    for _ in range(repetitions):
        try:
            # Запускаем программу
            cmd = ["./search", str(threads)]
            process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
                                     stderr=subprocess.PIPE, text=True)
            
            # Передаем данные через стандартный ввод
            input_data = f"{text}\n{pattern}\n"
            stdout, stderr = process.communicate(input=input_data, timeout=30)
            
            if process.returncode != 0:
                print(f"Ошибка выполнения: {stderr}")
                continue
                
            # Парсим результат
            for line in stdout.split('\n'):
                if line.startswith("RESULTS_CSV:"):
                    parts = line.replace("RESULTS_CSV:", "").strip().split(',')
                    if len(parts) >= 5:
                        parsed_length, pat_len, num_threads, time, found_matches = parts
                        times.append(float(time))
                        matches = int(found_matches)
                    break
                    
        except subprocess.TimeoutExpired:
            print(f"Таймаут для {threads} потоков")
            times.append(float('inf'))
        except Exception as e:
            print(f"Ошибка: {e}")
            times.append(float('inf'))
    
    if not times or all(t == float('inf') for t in times):
        return None
        
    return {
        'text_length': len(text),
        'pattern_length': len(pattern),
        'threads': threads,
        'time': np.mean(times),
        'matches': matches
    }

def main():
    # Тестовые данные
    test_cases = [

        # ("a" * 1000_000 + "abc" + "b" * 1000_000, "abc", "2 ляма"),
        ("a" * 10_000_000 + "abc" + "b" * 10_000_000, "abc", "20 mln"),
        ("a" * 25_000_000 + "abc" + "b" * 25_000_000, "abc", "50 mln"),
        ("a" * 50_000_000 + "abc" + "b" * 50_000_000, "abc", "100 mln"),
        ("a" * 250_000_000 + "abc" + "b" * 250_000_000, "abc", "500 mln"),
        # ("a" * 500_000_000 + "abc" + "b" * 500_000_000, "abc", "1 bill"),
        
    ]
    
    all_results = []
    
    for text, pattern, description in test_cases:
        print(f"\n{'='*50}")
        print(f"ТЕСТ: {description}")
        print(f"Длина: {len(text)}, Образец: '{pattern}'")
        print(f"{'='*50}")
        
        results = []
        # Тестируем 1, 2, 4, 6, 8 потоков
        threads_to_test = [1, 2, 4, 6, 8, 12, 16, 32]
        
        for threads in threads_to_test:
            print(f"Запуск с {threads} потоками...", end=' ')
            result = run_experiment(text, pattern, threads, repetitions=2)
            if result:
                results.append(result)
                print(f"Время: {result['time']:.6f}с, Найдено: {result['matches']}")
            else:
                print("Ошибка выполнения")
        
        if results:
            df = pd.DataFrame(results)
            # Рассчитываем ускорение и эффективность
            base_time = df[df['threads'] == 1]['time'].values[0]
            df['speedup'] = base_time / df['time']
            df['efficiency'] = df['speedup'] / df['threads']
            df['test_case'] = description
            
            all_results.append(df)
            
            # Выводим результаты для этого теста
            print(f"\nРезультаты для '{description}':")
            for _, row in df.iterrows():
                print(f"  Потоков: {row['threads']:2d} | "
                      f"Время: {row['time']:8.6f}с | "
                      f"Ускорение: {row['speedup']:5.2f}x | "
                      f"Эффективность: {row['efficiency']:6.1%}")
    
    # Сохраняем все результаты
    if all_results:
        final_df = pd.concat(all_results, ignore_index=True)
        final_df.to_csv('performance_results.csv', index=False)
        print(f"\nВсе результаты сохранены в 'performance_results.csv'")
        
        # Строим графики
        plot_results(final_df)
    else:
        print("Нет данных для анализа")

def plot_results(df):
    """Строит графики результатов с равномерным расположением потоков по X"""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    markers = ['o', 's', 'D', 'X', '^', 'v']
    unique_tests = list(df['test_case'].unique())

    threads_to_plot = [1, 2, 4, 6, 8, 12, 16, 32]
    x_labels = [str(t) for t in threads_to_plot]  # метки по X

    # === Время выполнения ===
    ax1 = axes[0, 0]
    for i, test_case in enumerate(unique_tests):
        case_data = df[df['test_case'] == test_case]
        y = [case_data[case_data['threads']==t]['time'].values[0] for t in threads_to_plot]
        ax1.plot(range(len(threads_to_plot)), y,
                 marker=markers[i % len(markers)],
                 linewidth=2, label=test_case)
    ax1.set_xticks(range(len(threads_to_plot)))
    ax1.set_xticklabels(x_labels)
    ax1.set_xlabel('Количество потоков')
    ax1.set_ylabel('Время (секунды)')
    ax1.set_title('Время выполнения поиска')
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    # === Ускорение ===
    ax2 = axes[0, 1]
    for i, test_case in enumerate(unique_tests):
        case_data = df[df['test_case'] == test_case]
        y = [case_data[case_data['threads']==t]['speedup'].values[0] for t in threads_to_plot]
        ax2.plot(range(len(threads_to_plot)), y,
                 marker=markers[i % len(markers)],
                 linewidth=2, label=test_case)
    #ax2.plot(range(len(threads_to_plot)), range(1,len(threads_to_plot)+1), 'k--', label='Идеальное ускорение')
    ax2.set_xticks(range(len(threads_to_plot)))
    ax2.set_xticklabels(x_labels)
    ax2.set_xlabel('Количество потоков')
    ax2.set_ylabel('Ускорение')
    ax2.set_title('Ускорение (Speedup)')
    ax2.legend()
    ax2.grid(True, alpha=0.3)

    # === Эффективность ===
    ax3 = axes[1, 0]
    for i, test_case in enumerate(unique_tests):
        case_data = df[df['test_case'] == test_case]
        y = [case_data[case_data['threads']==t]['efficiency'].values[0] for t in threads_to_plot]
        ax3.plot(range(len(threads_to_plot)), y,
                 marker=markers[i % len(markers)],
                 linewidth=2, label=test_case)
    ax3.set_xticks(range(len(threads_to_plot)))
    ax3.set_xticklabels(x_labels)
    ax3.set_xlabel('Количество потоков')
    ax3.set_ylabel('Эффективность')
    ax3.set_title('Эффективность потоков')
    ax3.legend()
    ax3.grid(True, alpha=0.3)

    # === Сводная таблица ===
    ax4 = axes[1, 1]
    ax4.axis('off')
    summary_text = "СВОДКА РЕЗУЛЬТАТОВ:\n\n"
    for test_case in unique_tests:
        case_data = df[df['test_case'] == test_case]
        best_idx = case_data['speedup'].idxmax()
        best_row = case_data.loc[best_idx]
        summary_text += f"{test_case}:\n"
        summary_text += f"  Лучшее ускорение: {best_row['speedup']:.2f}x\n"
        summary_text += f"  При {best_row['threads']} потоках\n"
        summary_text += f"  Эффективность: {best_row['efficiency']:.1%}\n\n"
    ax4.text(0.1, 0.9, summary_text, transform=ax4.transAxes,
             fontsize=10, verticalalignment='top', family='monospace')

    plt.tight_layout()
    plt.savefig('performance_analysis.png', dpi=300, bbox_inches='tight')
    plt.show()



if __name__ == "__main__":
    # Сначала компилируем программу
    print("🔧 Компилируем программу...")
    compile_result = subprocess.run(["gcc", "-pthread", "-o", "search", "search.c", "-O3"], 
                                  capture_output=True, text=True)
    if compile_result.returncode != 0:
        print("Ошибка компиляции:")
        print(compile_result.stderr)
        exit(1)
    
    print("✅ Компиляция успешна!")
    
    # Тестируем ручной ввод
    print("\n🧪 Тестируем ручной ввод...")
    test_process = subprocess.run(["./search", "2"], 
                                input="hello world hello\nhello\n", 
                                text=True, capture_output=True)
    print("Вывод программы:")
    print(test_process.stdout)
    if test_process.stderr:
        print("Ошибки:")
        print(test_process.stderr)
    
    main()