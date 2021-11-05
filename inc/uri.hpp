#ifndef URI_HPP
#define URI_HPP

#include <string>

namespace tristan::network{

    class Uri{

      public:
        Uri();
        explicit Uri(const std::string& url);
        Uri(const Uri& other) = default;
        Uri(Uri&& other) = default;

        Uri& operator=(const Uri& other) = default;
        Uri& operator=(Uri&& other) = default;

        ~Uri() = default;

        void setScheme(const std::string& scheme);
        void setAuthority(const std::string& host, const std::string& user_name = "", const std::string& user_password = "");
        void setPort(uint16_t port);
        void setPath(const std::string& path);
        void setQuery(const std::string& query);
        void setFragment(const std::string& fragment);

        [[nodiscard]] auto scheme() const -> const std::string&{ return m_scheme; }

        [[nodiscard]] auto userName() const -> const std::string&{ return m_user_name; }

        [[nodiscard]] auto userPassword() const -> const std::string&{ return m_user_password; }

        [[nodiscard]] auto host() const -> const std::string&{ return m_host; }

        [[nodiscard]] auto port() const -> const std::string&{ return m_port; }

        [[nodiscard]] auto path() const -> const std::string&{ return m_path; }

        [[nodiscard]] auto query() const -> const std::string&{ return m_query; }

        [[nodiscard]] auto fragment() const -> const std::string&{ return m_fragment; }

        [[nodiscard]] auto composeURI() const -> std::string;

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
