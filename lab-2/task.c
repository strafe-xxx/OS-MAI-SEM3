#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define ASCENDING 1
#define DESCENDING 0

int *array;
int array_size;
int max_threads;

// Структура для передачи данных потоку
typedef struct {
    int low;
    int count;
    int direction;
} ThreadData;

// Функция для обмена элементов
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Последовательная битоническая сортировка
void bitonic_merge(int low, int count, int direction) {
    if (count > 1) {
        int mid = count / 2;
        for (int i = low; i < low + mid; i++) {
            if ((direction == ASCENDING && array[i] > array[i + mid]) ||
                (direction == DESCENDING && array[i] < array[i + mid])) {
                swap(&array[i], &array[i + mid]);
            }
        }
        bitonic_merge(low, mid, direction);
        bitonic_merge(low + mid, mid, direction);
    }
}

// Функция для потока, реализующая битоническую сортировку
void *bitonic_sort(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int low = data->low;
    int count = data->count;
    int direction = data->direction;

    if (count > 1) {
        int mid = count / 2;

        ThreadData left_data = {low, mid, ASCENDING};
        ThreadData right_data = {low + mid, mid, DESCENDING};

        pthread_t left_thread, right_thread;

        // Создаём потоки для сортировки левой и правой частей
        if (max_threads > 1) {
            pthread_create(&left_thread, NULL, bitonic_sort, &left_data);
            pthread_create(&right_thread, NULL, bitonic_sort, &right_data);

            pthread_join(left_thread, NULL);
            pthread_join(right_thread, NULL);
        } else {
            bitonic_sort(&left_data);
            bitonic_sort(&right_data);
        }

        // Объединение битонических последовательностей
        bitonic_merge(low, count, direction);
    }

    return NULL;
}

// Функция проверки, отсортирован ли массив
int is_sorted(int *arr, int size) {
    for (int i = 0; i < size - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0; // Массив не отсортирован
        }
    }
    return 1; // Массив отсортирован
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <array_size> <max_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    array_size = atoi(argv[1]);
    max_threads = atoi(argv[2]);

    // Выделяем память для массива
    array = (int *)malloc(array_size * sizeof(int));
    if (array == NULL) {
        perror("Failed to allocate memory");
        return EXIT_FAILURE;
    }

    // Заполняем массив случайными числами
    srand(time(NULL));
    for (int i = 0; i < array_size; i++) {
        array[i] = rand() % 100;
    }

    // Измерение времени начала сортировки
    clock_t start_time = clock();

    // Инициализация и запуск сортировки
    ThreadData data = {0, array_size, ASCENDING};
    pthread_t main_thread;

    pthread_create(&main_thread, NULL, bitonic_sort, &data);
    pthread_join(main_thread, NULL);

    // Измерение времени завершения сортировки
    clock_t end_time = clock();

    // Проверка, отсортирован ли массив
    int sorted = is_sorted(array, array_size);

    // Вывод результата проверки
    if (sorted) {
        printf("Отсортирован массив: да\n");
    } else {
        printf("Отсортирован массив: нет\n");
    }

    // Вычисление и вывод времени выполнения
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Время: %.6f секунд\n", elapsed_time);

    free(array);
    return 0;
}
