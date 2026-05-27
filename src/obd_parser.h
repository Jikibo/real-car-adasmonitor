#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct OBDRecord {
    double speed_kmh{};
    double engine_rpm{};
    double throttle_pos{};
    double coolant_temp{};
    double fuel_level{};
    double intake_air_temp{};
    int label{}; // 0 = SLOW, 1 = NORMAL, 2 = AGGRESSIVE
};

/** \brief Парсер CSV-телеметрии автомобиля.
 *
 * Класс OBDParser отвечает за чтение данных из CSV файла, содержащего телеметрию автомобиля.
 */
class OBDParser {
public:
    /** \brief Конструктор класса.
     *
     * @param filename Путь к файлу с данными.
     */
    explicit OBDParser(std::string filename);
    /** \brief Загружает CSV-файл и возвращает число записей.
     *
     * @return Количество загруженных записей или -1 при ошибке.
     */
    int load();
        /** \brief Возвращает количество записей.
     *
     * @return Число записей.
     */
    std::size_t size() const noexcept;

    /** \brief Получает запись по индексу.
     *
     * @param index Индекс записи.
     * @return Ссылка на запись.
     */
    const OBDRecord& getRecord(int index) const;

    /** \brief Преобразует строковый тег в целое число.
     *
     * @param label Строковый тег ("SLOW", "NORMAL", "AGGRESSIVE").
     * @return Целочисленный код метки.
     */
    static int labelToInt(const std::string& label);
     /** \brief Преобразует целое число в строковый тег.
     *
     * @param label Целочисленный код метки.
     * @return Строковый тег.
     */
    static std::string intToLabel(int label);

private:
    std::string filename_;
    std::vector<OBDRecord> records_;

    /** \brief Удаляет пробельные символы в начале и конце строки.
     *
     * @param s Входная строка.
     * @return Обрезанная строка.
     */
    static std::string trim(const std::string& s);
    /** \brief Парсит одну строку CSV.
     *
     * @param line Строка данных.
     * @param record Запись для заполнения.
     * @param error Сообщение об ошибке при неудаче.
     * @return true, если парсинг успешен; false в противном случае.
     */
    bool parseLine(const std::string& line, OBDRecord& record, std::string& error) const;
};