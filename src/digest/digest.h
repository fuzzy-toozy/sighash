#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <stdexcept>

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/err.h>


#ifndef ENSURE
#    ifdef NDEBUG
#        define ENSURE(expr) do { if (! (expr)) { throw std::runtime_error(# expr); } } while (0)
#    else
#        define ENSURE(expr) assert(expr)
#    endif
#endif


namespace digest
{

    template <size_t TSize, typename TContext, int (*TInitFunc)(TContext *),
              int (*TUpdateFunc)(TContext *, const void *data, size_t len), int (*TFinalFunc)(unsigned char *md, TContext *)>
    class Digest final
    {
        using SelectedDigest = Digest<TSize, TContext, TInitFunc, TUpdateFunc, TFinalFunc>;

    public:
        using Result = std::array<unsigned char, TSize>;

        Digest()
        {
            ENSURE(TInitFunc(&m_openssl) == ERR_LIB_NONE);
        }

        Result final()
        {
            Result res;
            ENSURE(TFinalFunc(res.data(), &m_openssl) == ERR_LIB_NONE);
            return res;
        }

        void update(const void *data, size_t length)
        {
            ENSURE(TUpdateFunc(&m_openssl, data, length) == ERR_LIB_NONE);
        }

        std::string BytesToHEX(const uint8_t *buf, size_t len, bool uppercase = true)
        {
            std::ostringstream ss;

            if (uppercase)
            {
                ss << std::uppercase;
            }

            std::for_each(buf, buf + len, [&ss](auto byte)
                          { ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned>(byte); });

            return ss.str();
        }

    private:
        TContext m_openssl;
    };

    using DigestMD5 = Digest<MD5_DIGEST_LENGTH, MD5_CTX, MD5_Init, MD5_Update, MD5_Final>;

} //namespace digest
