#pragma once

#include <opencv2/opencv.hpp>
#include <string>

/** \brief Приборная панель для отображения телеметрии автомобиля.
 *
 * Класс Dashboard отвечает за рисование приборной панели с данными о скорости, оборотах и т.д.
 */
class Dashboard {
public:
    /** \brief Конструктор по умолчанию. */
    Dashboard() = default;

    /** \brief Рисует приборную панель на указанном изображении.
     *
     * @param frame Изображение для отрисовки.
     * @param speedKmh Скорость автомобиля (km/h).
     * @param rpm Обороты двигателя.
     * @param coolantTempC Температура охлаждающей жидкости.
     * @param fuelPercent Уровень топлива.
     * @param throttlePercent Положение дроссельной заслонки.
     * @param drivingStyle Стиль вождения (SLOW, NORMAL, AGGRESSIVE).
     */
    void draw(
        cv::Mat& frame,
        float speedKmh,
        float rpm,
        float coolantTempC,
        float fuelPercent,
        float throttlePercent,
        const std::string& drivingStyle
    ) const;

private:
    /** \brief Рисует гауге (круговую шкалу).
     *
     * @param img Изображение для рисования.
     * @param center Центр гауге.
     * @param radius Радиус.
     * @param value Значение на шкале.
     * @param minValue Минимальное значение.
     * @param maxValue Максимальное значение.
     * @param warningThreshold Порог предупреждения.
     * @param title Название шкалы.
     * @param unit Единица измерения.
     * @param normalColor Цвет для нормального значения.
     * @param warningColor Цвет для предупреждающего значения.
     */
    void drawGauge(
        cv::Mat& img,
        const cv::Point& center,
        int radius,
        float value,
        float minValue,
        float maxValue,
        float warningThreshold,
        const std::string& title,
        const std::string& unit,
        const cv::Scalar& normalColor,
        const cv::Scalar& warningColor
    ) const;

    /** \brief Рисует линейную шкалу.
     *
     * @param img Изображение для рисования.
     * @param area Прямоугольная область.
     * @param value Значение на шкале.
     * @param minValue Минимальное значение.
     * @param maxValue Максимальное значение.
     * @param title Название шкалы.
     * @param unit Единица измерения.
     * @param fillColor Цвет заполнения.
     */
    void drawLinearGauge(
        cv::Mat& img,
        const cv::Rect& area,
        float value,
        float minValue,
        float maxValue,
        const std::string& title,
        const std::string& unit,
        const cv::Scalar& fillColor
    ) const;

    /** \brief Возвращает цвет в зависимости от стиля вождения.
     *
     * @param drivingStyle Стиль вождения (SLOW, NORMAL, AGGRESSIVE).
     * @return Цвет для отображения.
     */
    static cv::Scalar styleColor(const std::string& drivingStyle);
};
