#ifndef ONNX_CLASSIFIER_H
#define ONNX_CLASSIFIER_H

#include <string>
#include <vector>
#include "obd_parser.h"

struct ClassificationResult {
    int label;         // 0=SLOW, 1=NORMAL, 2=AGGRESSIVE
    float confidence;
    std::vector<float> scores;
};

/** \brief ИИ-классификатор стиля вождения на основе ONNX модели.
 *
 * Класс ONNXClassifier загружает модель и выполняет классификацию телеметрии автомобиля.
 */
class ONNXClassifier {
public:
    /** \brief Конструктор класса.
     *
     * @param modelPath Путь к файлу модели ONNX.
     * @param paramsPath Путь к JSON с параметрами нормализации.
     */
    explicit ONNXClassifier(const std::string& modelPath, const std::string& paramsPath);

    /** \brief Выполняет классификацию записи телеметрии.
     *
     * @param record Запись для классификации.
     * @return Результат классификации.
     */
    ClassificationResult classify(const OBDRecord& record);

private:
    std::vector<float> mean_;
    std::vector<float> std_;

    /** \brief Читает параметры нормализации из JSON файла.
     *
     * @param path Путь к файлу с параметрами.
     */
    void readNormalizationParams(const std::string& path);
};

#endif
