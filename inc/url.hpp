#ifndef URL_HPP
#define URL_HPP

#include <string>

namespace tristan::network{
    /**
     * \class Url
     * \brief Implements Uniform Resource Identifier
     */
    class Url{

      public:
        /**
         * \brief Constructor
         * Sets \param m_valid to true
         */
        Url() noexcept;
        /**
         * \brief Overloaded constructor.
         * Parses passed uri string representation into values. If parsing was successful \param m_valid is set to true. URI representation must conform to the following format:
         * \par URI = scheme ":" ["//" authority] path ["?" query] ["#" fragment]
         * \param url const std::string& which represents and uri
         */
        explicit Url(const std::string& url);
        Url(const Url& other) = default;
        Url(Url&& other) noexcept = default;

        Url& operator=(const Url& other) = default;
        Url& operator=(Url&& other) noexcept = default;

        ~Url() = default;
        /**
         * \brief Sets scheme.
         * \param scheme const std::string&
         */
        void setScheme(const std::string& scheme);

        /**
         * \brief Sets host, user name and password which are parts of userinfo part of authority.
         * \param host const std::string&
         * \param user_name const std::string&. Default value is an empty string.
         * \param user_password const std::string&. Default value is an empty string.
         */
        void setAuthority(const std::string& host, const std::string& user_name = "", const std::string& user_password = "");

        /**
         * \brief Sets port.
         * \param port uint16_t
         */
        void setPort(uint16_t port);

        /**
         * \brief Sets port.
         * \param port uint16_t
         */
        void setPort(const std::string& port);

        /**
         * \brief Sets path
         * \param path const std::string&
         */
        void setPath(const std::string& path);

        /**
         * \brief Sets query.
         * \param query const std::string&
         */
        void setQuery(const std::string& query);

        /**
         * \brief Sets fragment.
         * \param fragment const std::string&
         */
        void setFragment(const std::string& fragment);

        /**
         * \brief Scheme getter.
         * \return const std::string&
         */
        [[nodiscard]] auto scheme() const -> const std::string&{ return m_scheme; }

        /**
         * \brief User name getter.
         * \return const std::string&
         */
        [[nodiscard]] auto userName() const -> const std::string&{ return m_user_name; }

        /**
         * \brief User password getter.
         * \return const std::string&
         */
        [[nodiscard]] auto userPassword() const -> const std::string&{ return m_user_password; }

        /**
         * \brief Host getter.
         * \return const std::string&
         */
        [[nodiscard]] auto host() const -> const std::string&{ return m_host; }

        /**
         * \brief Returns host in form of ip address
         * \return const std::string&
         */
        [[nodiscard]] auto hostIP() const -> const std::string&{ return m_host_ip; }

        /**
         * \brief Port getter.
         * \return Port or empty string.
         */
        [[nodiscard]] auto port() const -> const std::string&{ return m_port; }

        /**
        * \brief Path getter.
        * \return const std::string&
        */
        [[nodiscard]] auto path() const -> const std::string&{ return m_path; }

        /**
         * \brief Query getter.
         * \return const std::string&
         */
        [[nodiscard]] auto query() const -> const std::string&{ return m_query; }

        /**
         * \brief Fragment getter.
         * \return const std::string&
         */
        [[nodiscard]] auto fragment() const -> const std::string&{ return m_fragment; }

        /**
         * \brief Composes a string representation of URI.
         * \note IP address representation is considered as preferred.
         * \return std::string
         */
        [[nodiscard]] auto composeUrl(bool ip_address = true) const -> std::string;

        /**
         * \brief Check if URI is valid. Should be used in case of overloaded constructor.
         * \return True is valid and false otherwise.
         */
        [[nodiscard]] auto isValid() const -> bool{ return m_valid; }

      protected:

      private:

        std::string m_scheme;
        std::string m_user_name;
        std::string m_user_password;
        std::string m_host;
        std::string m_host_ip;
        std::string m_port;
        std::string m_path;
        std::string m_query;
        std::string m_fragment;

        bool m_valid;
    };

} //End of tristan::network namespace
#endif //URL_HPP
