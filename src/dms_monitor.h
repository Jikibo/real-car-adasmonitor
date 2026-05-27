#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

#include <deque>
#include <string>

/** \brief Состояние водителя.
 *
 * Структура содержит информацию о состоянии водителя, полученной от DMSMonitor.
 */
struct DriverState {
    /** \brief Флаг наличия лица на кадре. */
    bool face_detected = false;

    /** \brief Флаг открытости глаз. */
    bool eyes_open = false;

    /** \brief Флаг смотрит ли водитель прямо. */
    bool looking_forward = true;

    /** \brief Степень открытости глаз (0.0 - 1.0). */
    float eye_openness = 0.0f;

    /** \brief Угол поворота головы в градусах. */
    float head_turn_deg = 0.0f;

    /** \brief Алерт усталости (мечта). */
    bool alert_drowsy = false;

    /** \brief Алерт отвлечённости. */
    bool alert_distracted = false;

    /** \brief Координаты прямоугольника лица на кадре. */
    cv::Rect face_rect;
};

/** \brief Модуль детекции состояния водителя.
 *
 * Класс DMSMonitor анализирует кадры с камеры и определяет состояние водителя (усталость, отвлечение).
 */
class DMSMonitor {
public:
    /** \brief Конструктор класса.
     *
     * @param models_path Путь к папке с моделями DMS.
     */
    explicit DMSMonitor(const std::string& models_path);

    /** \brief Анализирует изображение и определяет состояние водителя.
     *
     * @param frame Входное изображение.
     * @return Состояние водителя (DriverState).
     */
    DriverState analyze(const cv::Mat& frame);

    /** \brief Проверяет готовность модуля к работе.
     *
     * @return true, если модели загружены успешно; false в противном случае.
     */
    bool isReady() const;

private:
    /** \brief Обнаруживает лицо на кадре.
     *
     * @param frame Входное изображение.
     * @param confidence Уровень уверенности (опционально).
     * @return Прямоугольник с координатами лица.
     */
    cv::Rect detectFace(const cv::Mat& frame, float* confidence = nullptr);

    /** \brief Оценивает степень открытости глаз.
     *
     * @param frame Входное изображение.
     * @param face_rect Прямоугольник с лицом.
     * @return Степень открытости (0.0 - 1.0).
     */
    float estimateEyeOpenness(const cv::Mat& frame, const cv::Rect& face_rect);

    /** \brief Оценивает угол поворота головы.
     *
     * @param frame Входное изображение.
     * @param face_rect Прямоугольник с лицом.
     * @return Угол поворота в градусах.
     */
    float estimateHeadTurn(const cv::Mat& frame, const cv::Rect& face_rect) const;

    /** \brief Обновляет историю состояния глаз.
     *
     * @param eyes_open Состояние открытости глаз.
     */
    void updateEyeHistory(bool eyes_open);

    /** \brief Проверяет признаки усталости.
     *
     * @return true, если водитель устал; false в противном случае.
     */
    bool isDrowsy() const;

private:
    cv::dnn::Net face_net_;          /**< Сеть для детекции лица. */
    cv::CascadeClassifier eye_cascade_;  /**< Каскадный классификатор глаз. */
    std::deque<bool> eye_history_;   /**< История состояния глаз. */

    float face_conf_threshold_ = 0.5f;               /**< Порог уверенности для лица. */
    float looking_forward_threshold_deg_ = 15.0f;   /**< Порог угла для "смотрит прямо". */

    static constexpr int kHistorySize = 45;         /**< Размер истории глаз (в кадрах). */
};
