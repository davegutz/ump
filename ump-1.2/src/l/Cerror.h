/* Stevens' error primitives */
/* 	$Id: Cerror.h,v 1.1 2006/08/16 22:07:35 davegutz Exp $	 */
void	 err_dump(const char *, ...);
void	 err_msg(const char *, ...);
void	 err_quit(const char *, ...);
void	 err_ret(const char *, ...);
void	 err_sys(const char *, ...);

