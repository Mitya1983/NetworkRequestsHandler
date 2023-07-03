#ifndef HTTP_PARAM_HPP
#define HTTP_PARAM_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace tristan::network {

    struct Parameter {
        std::string m_name;
        std::string m_string;

        Parameter(std::string name_, std::string value_) :
            m_name(std::move(name_)),
            m_string(std::move(value_)) { }
    };

    /**
     * \class HttpParams
     * \brief Handles HTTP base. That is this class stores header names and values.
     */
    class HttpParams {
    public:
        /**
         * \brief Default constructor
         */
        HttpParams() = default;
        /**
         * \brief Constructor
         * \param p_params_data const std::string&
         * \note application/x-www-form-urlencoded format is expected
         */
        explicit HttpParams(const std::string& p_params_data);
        /**
         * \brief Constructor
         * \param p_params_data const std::vector< uint8_t >&.
         * \note application/x-www-form-urlencoded format is expected
         */
        explicit HttpParams(const std::vector< uint8_t >& p_params_data);
        /**
         * \brief Copy constructor
         * \param p_other const HttpParams& other
         */
        HttpParams(const HttpParams& p_other) = default;
        /**
         * \brief Move constructor
         * \param p_other HttpParams&& other
         */
        HttpParams(HttpParams&& p_other) noexcept = default;
        /**
         * \brief Copy assignment operator
         * \param p_other const HttpParams& other
         * \return HttpParams&
         */
        HttpParams& operator=(const HttpParams& p_other) = default;
        /**
         * \brief Move assignment operator
         * \param p_other const HttpParams& other
         * \return HttpParams&
         */
        HttpParams& operator=(HttpParams&& p_other) noexcept = default;
        ~HttpParams() = default;

        void addParameter(Parameter&& p_parameter);

        /**
         * \brief Return value for specified header.
         * \param p_parameter_name std::optional< std::string >
         * \return Parameter value if parameter is present, std::nullopt otherwise.
         */
        [[nodiscard]] auto parameterValue(const std::string& p_parameter_name) const -> std::optional< std::string >;

        /**
         * \brief Returns if header list is empty
         * \return bool
         */
        [[nodiscard]] auto empty() -> bool;

        /**
         * \brief Returns iterator begin
         * \return std::vector< Parameter >::iterator
         */
        [[nodiscard]] auto begin() noexcept -> std::vector< Parameter >::iterator;

        /**
         * \brief Returns const_iterator begin
         * \return std::vector< Parameter >::const_iterator
         */
        [[nodiscard]] auto cbegin() const noexcept -> std::vector< Parameter >::const_iterator;

        /**
         * \brief Returns iterator end
         * \return std::vector< Parameter >::const_iterator
         */
        [[nodiscard]] auto end() noexcept -> std::vector< Parameter >::iterator;

        /**
         * \brief Returns const_iterator end
         * \return std::vector< Parameter >::const_iterator
         */
        [[nodiscard]] auto cend() const noexcept -> std::vector< Parameter >::const_iterator;

    private:
        std::vector< Parameter > m_params;
    };

}  // namespace tristan::network

#endif  //HTTP_PARAM_HPP
