/*	@file Executor.h
/*	@brief 任务执行者，跨windows和Linux平台。其中windows为vs2008环境，Linux为gcc4.9.0
/*
/*	@author liqilin
/*	@date	2016/12/2

/*	@note
/*	@note
/*	
/*	@warning
/*
 */

#include "typedefine.h"

namespace libext
{

class Executor
{
public:
	virtual ~Executor();
	virtual add(Func) = 0;
	virtual addWithPriority(Func, int8_t) {}
	virtual int8_t getNumPriority() { return 1; }

	static const int8_t LO_PRI = SCHAR_MIN;
	static const int8_t MID_PRI = 0;
	static const int8_t HI_PRI = SCHAR_MAX;

	template<class P>
	void addPtr(P f)
	{
#ifdef unix
		this->add([f]() {(*f)()});
#else
		this->add((Func)f);
#endif
	}

};


}//namespace libext

