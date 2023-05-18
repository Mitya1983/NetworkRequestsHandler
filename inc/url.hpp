#ifndef URL_HPP
#define URL_HPP

#include "network_error.hpp"

#include <string>
#include <vector>

struct IP {
    std::string as_string;
    uint32_t as_int;
};

namespace tristan::network {
    /**
     * \class Url
     * \brief Implements Uniform Resource Identifier
     */
    class Url {

    public:
        /**
         * \brief Constructor
         * Sets \param m_valid to true
         */
        Url() noexcept;
        /**
         * \brief Overloaded constructor.
         * Parses passed uri string representation into values. If parsing was successful
         * \param m_valid is set to true. URI representation must conform to the following format:
         * \par URI = scheme ":" ["//" authority] path ["?" query] ["#" fragment]
         * \param p_url const std::string& which represents and uri
         */
        explicit Url(const std::string& p_url);
        Url(const Url& other) = default;
        Url(Url&& other) noexcept = default;

        Url& operator=(const Url& other) = default;
        Url& operator=(Url&& other) noexcept = default;

        ~Url() = default;
        /**
         * \brief Sets scheme.
         * \param p_scheme const std::string&
         */
        void setScheme(const std::string& p_scheme);

        /**
         * \brief Sets host, user name and password which are parts of userinfo part of authority.
         * \param p_host const std::string&
         * \param p_user_name const std::string&. Default value is an empty string.
         * \param p_user_password const std::string&. Default value is an empty string.
         */
        void setAuthority(const std::string& p_host, const std::string& p_user_name = "", const std::string& p_user_password = "");

        /**
         * \brief Sets host IP
         * \param p_ip const std::string& ip
         */
        void addHostIP(const std::string& p_ip);

        /**
         * \brief Sets port.
         * \param p_port uint16_t
         */
        void setPort(uint16_t p_port);

        /**
         * \brief Sets port.
         * \param p_port const std::string&
         */
        void setPort(const std::string& p_port);

        /**
         * \brief Sets path
         * \param p_path const std::string&
         */
        void setPath(const std::string& p_path);

        /**
         * \brief Sets query.
         * \param p_query const std::string&
         */
        void setQuery(const std::string& p_query);

        /**
         * \brief Sets fragment.
         * \param p_fragment const std::string&
         */
        void setFragment(const std::string& p_fragment);

        /**
         * \brief Scheme getter.
         * \return const std::string&
         */
        [[nodiscard]] auto scheme() const noexcept -> const std::string&;

        /**
         * \brief User name getter.
         * \return const std::string&
         */
        [[nodiscard]] auto userName() const noexcept -> const std::string&;

        /**
         * \brief User password getter.
         * \return const std::string&
         */
        [[nodiscard]] auto userPassword() const noexcept -> const std::string&;

        /**
         * \brief Host getter.
         * \return const std::string&
         */
        [[nodiscard]] auto host() const noexcept -> const std::string&;

        /**
         * \brief Returns host in form of ip address
         * \return const std::string&
         */
        [[nodiscard]] auto hostIP() const noexcept -> IP;

        /**
         * \brief Returns list of host ip addresses
         * \return const std::vector<std::string>&
         */
        [[nodiscard]] auto hostIPList() const noexcept -> const std::vector< IP >&;

        /**
         * \brief Port getter.
         * \return Port or empty string.
         */
        [[nodiscard]] auto port() const noexcept -> const std::string&;

        /**
         * \brief Port getter in local byte order
         * \return uint16_t
         */
        [[nodiscard]] auto portUint16_t_local_byte_order() const noexcept -> uint16_t;

        /**
         * \brief Port getter in network byte order
         * \return uint16_t
         */
        [[nodiscard]] auto portUint16_t_network_byte_order() const noexcept -> uint16_t;

        /**
        * \brief Path getter.
        * \return const std::string&
        */
        [[nodiscard]] auto path() const noexcept -> const std::string&;

        /**
         * \brief Query getter.
         * \return const std::string&
         */
        [[nodiscard]] auto query() const noexcept -> const std::string&;

        /**
         * \brief Fragment getter.
         * \return const std::string&
         */
        [[nodiscard]] auto fragment() const noexcept -> const std::string&;

        /**
         * \brief Composes a string representation of URI.
         * \note IP address representation is considered as preferred.
         * \return std::string
         */
        [[nodiscard]] auto composeUrl() const -> std::string;

        /**
         * \brief Check if URI is valid. Should be used in case of overloaded constructor.
         * \return True is valid and false otherwise.
         */
        [[nodiscard]] auto isValid() const noexcept -> bool;

        /**
         * Retuns error if Url is in invalid state
         * @return std::error_code
         */
        [[nodiscard]] auto error() const noexcept -> std::error_code;

    protected:
    private:
        std::string m_scheme;
        std::string m_user_name;
        std::string m_user_password;
        std::string m_host;
        std::string m_port;
        std::string m_path;
        std::string m_query;
        std::string m_fragment;

        std::vector< IP > m_host_ip;

        std::error_code m_error;

        uint16_t m_port_local_byte_order;
        uint16_t m_port_network_byte_order;

        bool m_valid;

        void _resolveHost();
    };

}  // namespace tristan::network
#endif  //URL_HPP
