/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009-2013  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "scripting/flash/globalization/datetimeformatter.h"
#include "backends/locale.h"
#include "scripting/class.h"
#include "scripting/argconv.h"
#include "scripting/toplevel/Date.h"
#include <iomanip>

using namespace lightspark;

DateTimeFormatter::DateTimeFormatter(Class_base* c):
	ASObject(c)
{
}

void DateTimeFormatter::sinit(Class_base* c)
{
	CLASS_SETUP(c, ASObject, _constructor, CLASS_SEALED|CLASS_FINAL);
	REGISTER_GETTER(c, actualLocaleIDName);
	REGISTER_GETTER(c, lastOperationStatus);
	REGISTER_GETTER(c, requestedLocaleIDName);
	c->setDeclaredMethodByQName("setDateTimePattern","",Class<IFunction>::getFunction(c->getSystemState(),setDateTimePattern),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("format","",Class<IFunction>::getFunction(c->getSystemState(),format),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("formatUTC","",Class<IFunction>::getFunction(c->getSystemState(),formatUTC),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("getAvailableLocaleIDNames","",Class<IFunction>::getFunction(c->getSystemState(),formatUTC),NORMAL_METHOD,true);
}

ASFUNCTIONBODY_ATOM(DateTimeFormatter,_constructor)
{
	DateTimeFormatter* th =asAtomHandler::as<DateTimeFormatter>(obj);
	ARG_UNPACK_ATOM(th->requestedLocaleIDName)(th->dateStyle,"long")(th->timeStyle,"long");
	if (sys->localeManager->isLocaleAvailableOnSystem(th->requestedLocaleIDName.raw_buf()))
	{
		std::string localeName = sys->localeManager->getSystemLocaleName(th->requestedLocaleIDName.raw_buf());
		th->currlocale = std::locale(localeName);
		th->actualLocaleIDName = th->requestedLocaleIDName;
		th->lastOperationStatus="noError";
	}
	else
	{
		th->lastOperationStatus="usingDefaultWarning";
	}
}
ASFUNCTIONBODY_GETTER(DateTimeFormatter, actualLocaleIDName);
ASFUNCTIONBODY_GETTER(DateTimeFormatter, lastOperationStatus);
ASFUNCTIONBODY_GETTER(DateTimeFormatter, requestedLocaleIDName);

ASFUNCTIONBODY_ATOM(DateTimeFormatter,setDateTimePattern)
{
	DateTimeFormatter* th =asAtomHandler::as<DateTimeFormatter>(obj);
	ARG_UNPACK_ATOM(th->pattern);
	th->lastOperationStatus="noError";
}


ASFUNCTIONBODY_ATOM(DateTimeFormatter,format)
{
	DateTimeFormatter* th =asAtomHandler::as<DateTimeFormatter>(obj);
	_NR<Date> dt;
	ARG_UNPACK_ATOM(dt);
	tiny_string res;
	if (!dt.isNull())
	{
		LOG(LOG_NOT_IMPLEMENTED,"DateTimeFormatter.format is not really tested for all formats");
		std::locale l =  std::locale::global(th->currlocale);
		res = dt->format(th->pattern.raw_buf(),false);
		std::locale::global(l);
	}
	ret = asAtomHandler::fromString(sys,res);
}

ASFUNCTIONBODY_ATOM(DateTimeFormatter,formatUTC)
{
	DateTimeFormatter* th =asAtomHandler::as<DateTimeFormatter>(obj);
	_NR<Date> dt;
	ARG_UNPACK_ATOM(dt);
	tiny_string res;
	if (!dt.isNull())
	{
		LOG(LOG_NOT_IMPLEMENTED,"DateTimeFormatter.formatUTC is not really tested for all formats");
		std::locale l =  std::locale::global(th->currlocale);
		res = dt->format(th->pattern.raw_buf(),true);
		std::locale::global(l);
	}
	ret = asAtomHandler::fromString(sys,res);
}

ASFUNCTIONBODY_ATOM(DateTimeFormatter,getAvailableLocaleIDNames)
{
	DateTimeFormatter* th =asAtomHandler::as<DateTimeFormatter>(obj);
	Array* res=Class<Array>::getInstanceSNoArgs(sys);
	std::vector<std::string> localeIds = sys->localeManager->getAvailableLocaleIDNames();
	for (std::vector<std::string>::iterator it = localeIds.begin(); it != localeIds.end(); ++it)
	{
		tiny_string value = (*it);
		res->push(asAtomHandler::fromObject(abstract_s(sys, value)));
	}
	th->lastOperationStatus="noError";
	ret = asAtomHandler::fromObject(res);
}

