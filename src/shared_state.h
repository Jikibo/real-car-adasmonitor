#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <mutex>
#include <atomic>
#include "obd_parser.h"
#include "onnx_classifier.h"

/** \brief Разделяемое состояние между потоками.
 *
 * Класс SharedState содержит данные, которые используются в нескольких потоках.
 */
struct SharedState {
    /** \brief Текущая запись телеметрии из OBD. */
    OBDRecord current_record;

    /** \brief Результат классификации стиля вождения. */
    ClassificationResult classification_result;

    /** \brief Счётчик алертов. */
    int alert_count = 0;

    /** \brief Флаг выполнения программы. */
    std::atomic<bool> running{true};

    /** \brief Мьютекс для синхронизации доступа к данным. */
    mutable std::mutex mtx;

    // Конструктор
    SharedState() = default;
};

#endif // SHARED_STATE_H
