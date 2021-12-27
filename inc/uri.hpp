#ifndef URI_HPP
#define URI_HPP

#include <string>

namespace tristan::network{
    /**
     * \class Uri
     * \brief Implements Uniform Resource Identifier
     */
    class Uri{

      public:
        /**
         * \brief Constructor
         * Sets \param m_valid to true
         */
        Uri() noexcept;
        /**
         * \brief Overloaded constructor.
         * Parses passed uri string representation into values. If parsing was successful \param m_valid is set to true. URI representation must conform to the following format:
         * \par URI = scheme ":" ["//" authority] path ["?" query] ["#" fragment].
         * \param url const std::string& which represents and uri
         */
        explicit Uri(const std::string& url);
        Uri(const Uri& other) = default;
        Uri(Uri&& other) noexcept = default;

        Uri& operator=(const Uri& other) = default;
        Uri& operator=(Uri&& other) noexcept = default;

        ~Uri() = default;
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
         * \return Scheme or empty string.
         */
        [[nodiscard]] auto scheme() const -> const std::string&{ return m_scheme; }

        /**
         * \brief User name getter.
         * \return User name or empty string.
         */
        [[nodiscard]] auto userName() const -> const std::string&{ return m_user_name; }

        /**
         * \brief User password getter.
         * \return User password or empty string.
         */
        [[nodiscard]] auto userPassword() const -> const std::string&{ return m_user_password; }

        /**
         * \brief Host getter.
         * \return Host or empty string.
         */
        [[nodiscard]] auto host() const -> const std::string&{ return m_host; }

        /**
         * \brief Port getter.
         * \return Port or empty string.
         */
        [[nodiscard]] auto port() const -> const std::string&{ return m_port; }

        /**
        * \brief Path getter.
        * \return Path or empty string.
        */
        [[nodiscard]] auto path() const -> const std::string&{ return m_path; }
        /**
         * \brief Query getter.
         * \return Query or empty string.
         */
        [[nodiscard]] auto query() const -> const std::string&{ return m_query; }
        /**
         * \brief Fragment getter.
         * \return Fragment or empty string.
         */
        [[nodiscard]] auto fragment() const -> const std::string&{ return m_fragment; }
        /**
         * \brief Composes a string representation of URI.
         * \return Composed URI.
         */
        [[nodiscard]] auto composeUri() const -> std::string;
        /**
         * \brief Check if URI is valid. Should be used in cale of overloaded constructor.
         * \return True is valid and false otherwise.
         */
        [[nodiscard]] auto isValid() const -> bool{ return m_valid; }

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

        bool m_valid;
    };

} //End of tristan::network namespace
#endif //URI_HPP
