#ifndef EXCEPTIONS_HPP_ZLDCNT1I
#define EXCEPTIONS_HPP_ZLDCNT1I

#include <exception>
#include <string>

namespace kmq {

  class kmq_error : public std::exception
  {
    public:
      kmq_error(const std::string& msg) noexcept;
      const char* what() const noexcept override;
      virtual std::string name() const noexcept;
    private:
      std::string m_msg;
  };

  class kmq_io_error : public kmq_error
  {
    using kmq_error::kmq_error;
    virtual std::string name() const noexcept override;
  };

  class kmq_invalid_request : public kmq_error
  {
    using kmq_error::kmq_error;
    virtual std::string name() const noexcept override;
  };

  class kmq_invalid_index : public kmq_error
  {
    using kmq_error::kmq_error;
    virtual std::string name() const noexcept override;
  };
}

#endif /* end of include guard: EXCEPTIONS_HPP_ZLDCNT1I */
