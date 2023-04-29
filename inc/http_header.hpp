#ifndef HTTP_HEADER_HPP
#define HTTP_HEADER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace tristan::network {

    struct Header {
        std::string name;
        std::string value;

        Header(std::string name_, std::string value_) :
            name(std::move(name_)),
            value(std::move(value_)) { }
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
         * \param headers_data const std::string&
         */
        explicit HttpHeaders(const std::string& headers_data);
        /**
         * \brief Constructor
         * \param headers_data const std::vector< uint8_t >&.
         */
        explicit HttpHeaders(std::vector< uint8_t >&& headers_data);
        /**
         * \brief Copy constructor
         * \param other const HttpHeaders& other
         */
        HttpHeaders(const HttpHeaders& other) = default;
        /**
         * \brief Move constructor
         * \param other HttpHeaders&& other
         */
        HttpHeaders(HttpHeaders&& other) noexcept = default;
        /**
         * \brief Copy assignment operator
         * \param other const HttpHeaders& other
         * \return HttpHeaders&
         */
        HttpHeaders& operator=(const HttpHeaders& other) = default;
        /**
         * \brief Move assignment operator
         * \param other const HttpHeaders& other
         * \return HttpHeaders&
         */
        HttpHeaders& operator=(HttpHeaders&& other) noexcept = default;
        ~HttpHeaders() = default;

        void addHeader(Header&& header);

        /**
         * \brief Return value for specified header.
         * \param header_name std::optional< std::string >
         * \return Header value if header is present, std::nullopt otherwise.
         */
        [[nodiscard]] auto headerValue(const std::string& header_name) const -> std::optional< std::string >;

        /**
         * \brief Returns if header list is empty
         * \return bool
         */
        [[nodiscard]] auto empty() -> bool;

        /**
         * \brief Returns iterator begin
         * \return std::vector< Header >::iterator
         */
        [[nodiscard]] constexpr auto begin() noexcept -> std::vector< Header >::iterator;

        /**
         * \brief Returns const_iterator begin
         * \return std::vector< Header >::const_iterator
         */
        [[nodiscard]] constexpr auto cbegin() const noexcept -> std::vector< Header >::const_iterator;

        /**
         * \brief Returns iterator end
         * \return std::vector< Header >::const_iterator
         */
        [[nodiscard]] constexpr auto end() noexcept -> std::vector< Header >::iterator;


        /**
         * \brief Returns const_iterator end
         * \return std::vector< Header >::const_iterator
         */
        [[nodiscard]] constexpr auto cend() const noexcept -> std::vector< Header >::const_iterator;

    private:
        std::vector< Header > m_headers;
    };

}  // namespace tristan::network

#endif  // HTTP_HEADER_HPP
