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

#include "locale.h"

#include <locale>

using namespace lightspark;
using namespace std;

LocaleManager::LocaleManager()
{
    locales["en_AU"] = new LocaleItem("english", "Australia", "", "", "", false);
    locales["en_CA"] = new LocaleItem("english", "Canada", "", "", "", false);
    locales["en_DK"] = new LocaleItem("english", "Denmark", "", "", "", false);
    locales["en_GB"] = new LocaleItem("english", "United Kingdom", "", "", "", false);
    locales["en_IE"] = new LocaleItem("english", "Ireland", "", "", "", false);
    locales["en_IN"] = new LocaleItem("english", "India", "", "", "", false);
    locales["en_NZ"] = new LocaleItem("english", "New Zealand", "", "", "", false);
    locales["en_PH"] = new LocaleItem("english", "Philippines", "", "", "", false);
    locales["en_US"] = new LocaleItem("english", "United States", "", "", "", false);
    locales["en_ZA"] = new LocaleItem("english", "South Africa", "", "", "", false);
}

LocaleManager::~LocaleManager()
{
    for (std::unordered_map<string, LocaleItem*>::iterator it = locales.begin(); it != locales.end(); ++it)
    {
        delete (*it).second;
    }
}

bool LocaleManager::isLocaleAvailableOnSystem(std::string locale)
{
    try
	{
		std::locale(locale);
        return true;
	}
	catch (std::runtime_error& e)
	{
		uint32_t pos = locale.find("-");
		if(pos != locale.size()) // Check this line
		{
			std::string r("_");
			std::string l = locale.replace(pos,1,r);
			try
			{
				// try with "_" instead of "-"
				std::locale(l);
				return true;
			}
			catch (std::runtime_error& e)
			{
				try
				{
					// try appending ".UTF-8"
					l += ".UTF-8";
					std::locale(l);
					return true;
				}
				catch (std::runtime_error& e)
				{
                    //LOG(LOG_INFO,"unknown locale:"<<th->requestedLocaleIDName<<" "<<e.what());
					return false;
				}
			}
		}
		else
		{
			try
			{
				// try appending ".UTF-8"
				locale += ".UTF-8";
				std::locale(locale);
				return true;
			}
			catch (std::runtime_error& e)
			{
                //LOG(LOG_INFO,"unknown locale:"<<th->requestedLocaleIDName<<" "<<e.what());
				return false;
			}
		}
	}
    return true;
}

std::string LocaleManager::getSystemLocaleName(std::string name)
{
    try
	{
		std::locale(name);
        return name.name();
	}
	catch (std::runtime_error& e)
	{
		uint32_t pos = name.find("-");
		if(pos != name.size()) // Check this line
		{
			std::string r("_");
			std::string l = name.replace(pos,1,r);
			try
			{
				// try with "_" instead of "-"
				std::locale(l);
				return l.name();
			}
			catch (std::runtime_error& e)
			{
				try
				{
					// try appending ".UTF-8"
					l += ".UTF-8";
					std::locale(l);
					return l.name();
				}
				catch (std::runtime_error& e)
				{
                    //LOG(LOG_INFO,"unknown locale:"<<th->requestedLocaleIDName<<" "<<e.what());
					return "";
				}
			}
		}
		else
		{
			try
			{
				// try appending ".UTF-8"
				name += ".UTF-8";
				std::locale(name);
				return name.name();
			}
			catch (std::runtime_error& e)
			{
                //LOG(LOG_INFO,"unknown locale:"<<th->requestedLocaleIDName<<" "<<e.what());
				return "";
			}
		}
	}
    return "";
}

LocaleItem* LocaleManager::getLocaleId(std::string name)
{
    return locales[name];
}

std::vector<std::string> LocaleManager::getAvailableLocaleIDNames()
{
    std::vector<std::string> values;
    for (auto it = locales.begin(); it != locales.end(); ++it)
    {
        if (isLocaleAvailableOnSystem((*it).first))
        {
            values.push_back((*it).first);
        }
    }
    return values;
}