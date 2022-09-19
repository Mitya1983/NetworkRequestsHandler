#ifndef HTTP_PARAM_HPP
#define HTTP_PARAM_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace tristan::network {

    struct Parameter {
        std::string name;
        std::string value;

        Parameter(std::string name_, std::string value_) :
            name(std::move(name_)),
            value(std::move(value_)) { }
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
         * \param params_data const std::string&
         * \note application/x-www-form-urlencoded format is expected
         */
        explicit HttpParams(const std::string& params_data);
        /**
         * \brief Constructor
         * \param params_data const std::vector< uint8_t >&.
         * \note application/x-www-form-urlencoded format is expected
         */
        explicit HttpParams(const std::vector< uint8_t >& params_data);
        /**
         * \brief Copy constructor
         * \param other const HttpParams& other
         */
        HttpParams(const HttpParams& other) = default;
        /**
         * \brief Move constructor
         * \param other HttpParams&& other
         */
        HttpParams(HttpParams&& other) noexcept = default;
        /**
         * \brief Copy assignment operator
         * \param other const HttpParams& other
         * \return HttpParams&
         */
        HttpParams& operator=(const HttpParams& other) = default;
        /**
         * \brief Move assignment operator
         * \param other const HttpParams& other
         * \return HttpParams&
         */
        HttpParams& operator=(HttpParams&& other) noexcept = default;
        ~HttpParams() = default;

        void addParameter(Parameter&& parameter);

        /**
         * \brief Return value for specified header.
         * \param parameter_name std::optional< std::string >
         * \return Parameter value if parameter is present, std::nullopt otherwise.
         */
        [[nodiscard]] auto parameterValue(const std::string& parameter_name) const
            -> std::optional< std::string >;

        /**
         * \brief Returns if header list is empty
         * \return bool
         */
        [[nodiscard]] auto empty() -> bool;

        /**
         * \brief Returns iterator begin
         * \return std::vector< Header >::iterator
         */
        [[nodiscard]] constexpr auto begin() noexcept -> std::vector< Parameter >::iterator {
            return m_params.begin();
        }

        /**
         * \brief Returns const_iterator begin
         * \return std::vector< Header >::const_iterator
         */
        [[nodiscard]] constexpr auto cbegin() const noexcept -> std::vector< Parameter >::const_iterator{
            return m_params.cbegin();
        }

        /**
         * \brief Returns iterator end
         * \return std::vector< Header >::const_iterator
         */
        [[nodiscard]] constexpr auto end() noexcept -> std::vector< Parameter >::iterator{
            return m_params.end();
        }


        /**
         * \brief Returns const_iterator end
         * \return std::vector< Header >::const_iterator
         */
        [[nodiscard]] constexpr auto cend() const noexcept -> std::vector< Parameter >::const_iterator{
            return m_params.cend();
        }

    private:
        std::vector< Parameter > m_params;
    };

}  // namespace tristan::network

#endif  //HTTP_PARAM_HPP
