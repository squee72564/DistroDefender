#include <geo/downloader.hpp>

#include <algorithm>
#include <iomanip>

#include <openssl/evp.h>
#include <cpr/cpr.h>

#include <fmt/format.h>

#include <sstream>
namespace
{
[[maybe_unused]] std::size_t writeCallback(void* contents, std::size_t size, std::size_t nmemb, std::string* userp)
{
    userp->append(static_cast<char*>(contents), size * nmemb);

    return size * nmemb;
}

bool isMD5Hash(const std::string_view str)
{
    return str.size() == 32 && std::all_of(str.cbegin(), str.cend(), ::isxdigit);
}
} // namespace


namespace geo
{

base::RespOrError<std::string> Downloader::downloadHTTPS(std::string_view url) const
{
    cpr::Response response = cpr::Get(cpr::Url{std::string{url}});

    if (response.error || response.status_code < 200 | response.status_code >= 300)
    {
        return base::Error{
            fmt::format(
                "Failed to download file from '{}'. error {}, status code: {}.",
                url, response.error.message, response.status_code
            )
        };
    }

    return response.text;
}

base::RespOrError<std::string> Downloader::computeMD5(std::string_view data) const
{
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), &EVP_MD_CTX_free);

    if (!ctx)
    {
        return base::Error{
            "Failed to allocate EVP_MD_CTX"
        };
    }

    if (EVP_DigestInit_ex(ctx.get(), EVP_md5(), nullptr) != 1)
    {
        return base::Error{
            "EVP_DigestInit_ex failed"
        };
    }

    if (EVP_DigestUpdate(ctx.get(), data.data(), data.size()) != 1)
    {
        return base::Error{
            "EVP_DigestUpdate_ex failed"
        };
    }

    unsigned char digest[EVP_MAX_MD_SIZE] = {0};
    unsigned int digest_len{0};

    if (EVP_DigestFinal_ex(ctx.get(), digest, &digest_len) != 1)
    {
        return base::Error {
            "EVP_DigestFinal_ex Failed"
        };
    }

    std::stringstream ss;
    for (unsigned int i = 0; i < digest_len; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }

    return ss.str();
}

base::RespOrError<std::string> Downloader::downloadMD5(std::string_view url) const
{
    auto response = downloadHTTPS(url);

    if (base::isError(response))
    {
        return base::getError(response);
    }

    auto hash = base::getResponse(response);

    while (!hash.empty() && hash[hash.size()-1] == '\n')
    {
        hash.pop_back();
    }

    if (!isMD5Hash(hash))
    {
        return base::Error{
            fmt::format(
                "Invalid MD5 hash: '{}'",
                hash
            )
        };
    }

    return hash;
}

} // namespace geo
