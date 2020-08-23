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

#include "scripting/flash/globalization/collator.h"
#include "scripting/class.h"
#include "scripting/argconv.h"
#include <iomanip>

using namespace lightspark;

void Collator::sinit(Class_base* c)
{
	CLASS_SETUP_NO_CONSTRUCTOR(c, ASObject, CLASS_SEALED|CLASS_FINAL);

    REGISTER_GETTER(c, actualLocaleIDName);
    REGISTER_GETTER_SETTER(c, ignoreCase);
	REGISTER_GETTER_SETTER(c, ignoreCharacterWidth);
	REGISTER_GETTER_SETTER(c, ignoreDiacritics);
	REGISTER_GETTER_SETTER(c, ignoreKanaType);
	REGISTER_GETTER_SETTER(c, ignoreSymbols);
	REGISTER_GETTER(c, lastOperationStatus);
    REGISTER_GETTER_SETTER(c, numericComparison);
	REGISTER_GETTER(c, requestedLocaleIDName);

	c->setDeclaredMethodByQName("compare","",Class<IFunction>::getFunction(c->getSystemState(),compare),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("equals","",Class<IFunction>::getFunction(c->getSystemState(),equals),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("getAvailableLocaleIDNames","",Class<IFunction>::getFunction(c->getSystemState(),getAvailableLocaleIDNames),NORMAL_METHOD,true);
}

ASFUNCTIONBODY_ATOM(Collator,_constructor)
{
	Collator* th =asAtomHandler::as<Collator>(obj);

	ARG_UNPACK_ATOM(th->requestedLocaleIDName);
	ARG_UNPACK_ATOM(th->initialMode);

	try
	{
		th->currlocale = std::locale(th->requestedLocaleIDName.raw_buf());
		th->actualLocaleIDName = th->requestedLocaleIDName;
		th->lastOperationStatus="noError";
	}
	catch (std::runtime_error& e)
	{
		uint32_t pos = th->requestedLocaleIDName.find("-");
		if(pos != tiny_string::npos)
		{
			tiny_string r("_");
			tiny_string l = th->requestedLocaleIDName.replace(pos,1,r);
			try
			{
				// try with "_" instead of "-"
				th->currlocale = std::locale(l.raw_buf());
				th->actualLocaleIDName = th->requestedLocaleIDName;
				th->lastOperationStatus="noError";
			}
			catch (std::runtime_error& e)
			{
				try
				{
					// try appending ".UTF-8"
					l += ".UTF-8";
					th->currlocale = std::locale(l.raw_buf());
					th->actualLocaleIDName = th->requestedLocaleIDName;
					th->lastOperationStatus="noError";
				}
				catch (std::runtime_error& e)
				{
					th->lastOperationStatus="usingDefaultWarning";
					LOG(LOG_ERROR,"unknown locale:"<<th->requestedLocaleIDName<<" "<<e.what());
				}
			}
		}
		else
		{
			try
			{
				// try appending ".UTF-8"
				th->requestedLocaleIDName += ".UTF-8";
				th->currlocale = std::locale(th->requestedLocaleIDName.raw_buf());
				th->actualLocaleIDName = th->requestedLocaleIDName;
				th->lastOperationStatus="noError";
			}
			catch (std::runtime_error& e)
			{
				th->lastOperationStatus="usingDefaultWarning";
				LOG(LOG_ERROR,"unknown locale:"<<th->requestedLocaleIDName<<" "<<e.what());
			}
		}
	}
    th->lastOperationStatus = "no_error";
}

ASFUNCTIONBODY_GETTER(Collator, actualLocaleIDName);
ASFUNCTIONBODY_GETTER_SETTER(Collator, ignoreCase);
ASFUNCTIONBODY_GETTER_SETTER(Collator, ignoreCharacterWidth);
ASFUNCTIONBODY_GETTER_SETTER(Collator, ignoreDiacritics);
ASFUNCTIONBODY_GETTER_SETTER(Collator, ignoreKanaType);
ASFUNCTIONBODY_GETTER_SETTER(Collator, ignoreSymbols);
ASFUNCTIONBODY_GETTER(Collator, lastOperationStatus);
ASFUNCTIONBODY_GETTER_SETTER(Collator, numericComparison);
ASFUNCTIONBODY_GETTER(Collator, requestedLocaleIDName);

bool Collator::isSymbol(char character)
{
	// Space is a symbol in this API
	if (isspace(character) || character == ' ' || character == 32) 
	{
		return true;
	}
	if (iswpunct(character))
	{
		return true;
	}
	// Check for more symbols to add!!!
	return iswcntrl(character);
}

int Collator::compare(
	std::string string1,
	std::string string2,
	bool ignoreCase,
	bool ignoreCharacterWidth,
	bool ignoreDiacritics,
	bool ignoreKanaType,
	bool ignoreSymbols)
{
	string::iterator string1It = string1.begin();
    string::iterator string2It = string2.begin();
	string s1 = "";
	string s2 = "";
	while (string1It != string1.end())
	{
		char char1 = (*string1It);
		if (ignoreSymbols && isSymbol(char1))
		{

		}
		else if (ignoreCase)
		{
			char1 = std::tolower(char1);
			s1.push_back(char1);
		}
		else
		{
			s1.push_back(char1);
		}
		string1It++;
	}

	while (string2It != string2.end())
	{
		char char2 = (*string2It);
		if (ignoreSymbols && isSymbol(char2))
		{

		}
		else if (ignoreCase)
		{
			char2 = std::tolower(char2);
			s2.push_back(char2);
		}
		else
		{
			s2.push_back(char2);
		}
		string2It++;
	}
	int result = s2.compare(s1);
	if (result > 0)
	{
		return 1;
	}
	if (result < 0)
	{
		return -1;
	}
	return 0;
}

bool Collator::equals(
	std::string string1,
	std::string string2,
	bool ignoreCase,
	bool ignoreCharacterWidth,
	bool ignoreDiacritics,
	bool ignoreKanaType,
	bool ignoreSymbols)
{
	string::iterator string1It = string1.begin();
	string::iterator string2It = string2.begin();
	
	while (string1It != string1.end() && string2It != string2.end())
	{
		gunichar char1 = (*string1It);
		gunichar char2 = (*string2It);

		if (ignoreCase)
		{
			char1 = std::tolower(char1);
			char2 = std::tolower(char2);
		}

		if (ignoreSymbols && (isSymbol(char1) || isSymbol(char2)))
		{
			if (isSymbol(char1))
			{
				++string1It;
			}
			if (isSymbol(char2))
			{
				++string2It;
			}
		}
		else
		{
			if (char1 != char2)
			{
				return false;
			}
			++string1It;
			++string2It;
		}
	}
	if (string1It != string1.end())
	{
		return false;
	}
	if (string2It != string2.end())
	{
		return false;
	}
	return true;
}

ASFUNCTIONBODY_ATOM(Collator,compare)
{
    LOG(LOG_NOT_IMPLEMENTED,"Collator.compare is not really tested for all formats");
	Collator* th = asAtomHandler::as<Collator>(obj);
    if (th->ignoreKanaType)
	{
		LOG(LOG_NOT_IMPLEMENTED,"ignoreKanaType is not supported");
	}
    if (th->numericComparison)
	{
		LOG(LOG_NOT_IMPLEMENTED,"numericComparison is not supported");
	}
	if (th->ignoreDiacritics)
	{
		LOG(LOG_NOT_IMPLEMENTED,"diacritics is not supported");
	}
	if (th->ignoreCharacterWidth)
	{
		LOG(LOG_NOT_IMPLEMENTED,"ignoreCharacterWidth is not supported");
	}
	try
    {
		std::locale l =  std::locale::global(th->currlocale);
	    tiny_string string1;
	    tiny_string string2;
	    ARG_UNPACK_ATOM(string1)(string2);
		std::string s1 = string1.raw_buf();
		std::string s2 = string2.raw_buf();
	    std::locale::global(l);
	 	int value = 0;
		if (th->initialMode == "matching")
		{
			value = th->compare(s1, s2, true, true, true, true, true);
		}
		else
		{
			value = th->compare(s1, s2, th->ignoreCase, th->ignoreCharacterWidth, th->ignoreDiacritics, th->ignoreKanaType, th->ignoreSymbols);
		}
		ret = asAtomHandler::fromInt(value);
		th->lastOperationStatus = "noError";
    }
    catch (std::runtime_error& e)
    {
		th->lastOperationStatus="usingDefaultWarning";
		LOG(LOG_ERROR,"unknown locale:"<<th->requestedLocaleIDName<<" "<<e.what());
    }
}

ASFUNCTIONBODY_ATOM(Collator,equals)
{
	LOG(LOG_NOT_IMPLEMENTED,"Collator.equals is not really tested for all formats");
	Collator* th = asAtomHandler::as<Collator>(obj);
	if (th->ignoreKanaType)
	{
		LOG(LOG_NOT_IMPLEMENTED,"ignoreKanaType is not supported");
	}
    if (th->numericComparison)
	{
		LOG(LOG_NOT_IMPLEMENTED,"numericComparison is not supported");
	}
	if (th->ignoreDiacritics)
	{
		LOG(LOG_NOT_IMPLEMENTED,"diacritics is not supported");
	}
	if (th->ignoreCharacterWidth)
	{
		LOG(LOG_NOT_IMPLEMENTED,"ignoreCharacterWidth is not supported");
	}
	try
    {
		tiny_string string1;
	    tiny_string string2;
	    ARG_UNPACK_ATOM(string1)(string2);
		std::locale l =  std::locale::global(th->currlocale);
		std::string s1 = string1.raw_buf();
		std::string s2 = string2.raw_buf();
	    std::locale::global(l);
		bool value = false;
		if (th->initialMode == "matching")
		{
			value = th->equals(s1, s2, true, true, true, true, true);
		}
		else
		{
			value = th->equals(s1, s2, th->ignoreCase, th->ignoreCharacterWidth, th->ignoreDiacritics, th->ignoreKanaType, th->ignoreSymbols);
		}
		ret = asAtomHandler::fromBool(value);
		th->lastOperationStatus = "noError";
    }
    catch (std::runtime_error& e)
    {
		th->lastOperationStatus="usingDefaultWarning";
		LOG(LOG_ERROR,"unknown locale:"<<th->requestedLocaleIDName<<" "<<e.what());
    }
}

ASFUNCTIONBODY_ATOM(Collator,getAvailableLocaleIDNames)
{
	LOG(LOG_NOT_IMPLEMENTED,"Collator.getAvailableLocaleIDNames is not implemented.");
}