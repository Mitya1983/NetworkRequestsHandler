#ifndef HTTP_HEADER_HPP
#define HTTP_HEADER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace tristan::network {

    struct Header {
        std::string m_name;
        std::string m_string;

        Header(std::string name_, std::string value_) :
            m_name(std::move(name_)),
            m_string(std::move(value_)) { }
    };

    /**
     * \class HttpHeaders
     * \brief Handles HTTP base. That is this class stores header names and values.
     */
    class HttpHeaders {
    public:
        /**
         * \brief Default constructor
         */
        HttpHeaders() = default;
        /**
         * \brief Constructor
         * \param p_headers_data const std::string&
         */
        explicit HttpHeaders(const std::string& p_headers_data);
        /**
         * \brief Constructor
         * \param p_headers_data const std::vector< uint8_t >&.
         */
        explicit HttpHeaders(std::vector< uint8_t >&& p_headers_data);
        /**
         * \brief Copy constructor
         * \param p_other const HttpHeaders& other
         */
        HttpHeaders(const HttpHeaders& p_other) = default;
        /**
         * \brief Move constructor
         * \param p_other HttpHeaders&& other
         */
        HttpHeaders(HttpHeaders&& p_other) noexcept = default;
        /**
         * \brief Copy assignment operator
         * \param p_other const HttpHeaders& other
         * \return HttpHeaders&
         */
        HttpHeaders& operator=(const HttpHeaders& p_other) = default;
        /**
         * \brief Move assignment operator
         * \param p_other const HttpHeaders& other
         * \return HttpHeaders&
         */
        HttpHeaders& operator=(HttpHeaders&& p_other) noexcept = default;
        ~HttpHeaders() = default;

        void addHeader(Header&& p_header);

        /**
         * \brief Return value for specified header.
         * \param p_header_name std::optional< std::string >
         * \return Header value if header is present, std::nullopt otherwise.
         */
        [[nodiscard]] auto headerValue(const std::string& p_header_name) const -> std::optional< std::string >;

        /**
         * \brief Returns if header list is empty
         * \return bool
         */
        [[nodiscard]] auto empty() -> bool;

        /**
         * \brief Returns iterator begin
         * \return std::vector< Header >::iterator
         */
        [[nodiscard]] auto begin() noexcept -> std::vector< Header >::iterator;

        /**
         * \brief Returns const_iterator begin
         * \return std::vector< Header >::const_iterator
         */
        [[nodiscard]] auto cbegin() const noexcept -> std::vector< Header >::const_iterator;

        /**
         * \brief Returns iterator end
         * \return std::vector< Header >::const_iterator
         */
        [[nodiscard]] auto end() noexcept -> std::vector< Header >::iterator;


        /**
         * \brief Returns const_iterator end
         * \return std::vector< Header >::const_iterator
         */
        [[nodiscard]] auto cend() const noexcept -> std::vector< Header >::const_iterator;

    private:
        std::vector< Header > m_headers;
    };

}  // namespace tristan::network

#endif  // HTTP_HEADER_HPP
