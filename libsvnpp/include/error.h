#ifndef ERROR_H
#define ERROR_H

#define SVNPP_ERR(expr)                           \
  do {                                          \
    svn_error_t *svn_err__temp = (expr);        \
    if (svn_err__temp)                          \
        svn_error_trace(svn_err__temp);    \
  } while (0)


#endif // ERROR_H
