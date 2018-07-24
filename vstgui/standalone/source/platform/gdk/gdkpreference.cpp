// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkpreference.h"
#include "../../../include/iapplication.h"
#include "../../../include/icommondirectories.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {
namespace {

//------------------------------------------------------------------------
constexpr auto CreateTableSQL = R"__(
CREATE TABLE IF NOT EXISTS "store" (
    "key" TEXT NOT NULL PRIMARY KEY,
    "value" TEXT NOT NULL
)
)__";

//------------------------------------------------------------------------
constexpr auto GetValueSQL = R"__(SELECT "value" FROM "store" WHERE "key" IS )__";
constexpr auto SetValueSQL = R"__(INSERT INTO "store" VALUES )__";
constexpr auto DeleteValueSQL = R"__(DELETE FROM "store" WHERE key=)__";

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
Preference::Preference () {}

//------------------------------------------------------------------------
Preference::~Preference () noexcept
{
	if (db)
		sqlite3_close (db);
}

//------------------------------------------------------------------------
bool Preference::set (const UTF8String& key, const UTF8String& value)
{
	if (!prepare ())
		return false;
	if (get (key))
	{
		char* errorMsg = nullptr;
		std::string sql = DeleteValueSQL;
		sql += "\"" + key + "\"";
		sqlite3_exec (db, sql.data (), nullptr, nullptr, &errorMsg);
		if (errorMsg)
		{
			printf ("%s\n", errorMsg);
			sqlite3_free (errorMsg);
			return false;
		}
	}
	char* errorMsg = nullptr;
	std::string sql = SetValueSQL;
	sql += "(\"" + key + "\",\"" + value + "\")";
	sqlite3_exec (db, sql.data (), nullptr, nullptr, &errorMsg);
	if (errorMsg)
	{
		printf ("%s\n", errorMsg);
		sqlite3_free (errorMsg);
		return false;
	}

	return true;
}

//------------------------------------------------------------------------
Optional<UTF8String> Preference::get (const UTF8String& key)
{
	if (!prepare ())
		return {};
	std::string sql = GetValueSQL;
	sql += "\"" + key + "\"";
	UTF8String result;
	char* errorMsg = nullptr;
	sqlite3_exec (db, sql.data (),
				  [](void* userData, int numColumns, char** cols, char** colNames) -> int {
					  auto result = reinterpret_cast<UTF8String*> (userData);
					  if (numColumns > 0)
						  *result = cols[0];
					  return 0;
				  },
				  &result, &errorMsg);
	if (errorMsg)
	{
		printf ("%s\n", errorMsg);
		sqlite3_free (errorMsg);
		return {};
	}
	if (result.empty ())
		return {};
	return Optional<UTF8String> (std::move (result));
}

//------------------------------------------------------------------------
bool Preference::prepare ()
{
	if (db)
		return true;
	auto prefPath = IApplication::instance ().getCommonDirectories ().get (
		CommonDirectoryLocation::AppPreferencesPath, "", true);
	if (!prefPath)
		return false;
	*prefPath += "preferences.db";
	if (sqlite3_open (prefPath->data (), &db) != 0)
		return false;
	char* errorMsg = nullptr;
	sqlite3_exec (db, CreateTableSQL, [](void*, int, char**, char**) -> int { return 0; }, nullptr,
				  &errorMsg);
	if (errorMsg)
	{
		printf ("%s\n", errorMsg);
		sqlite3_free (errorMsg);
	}
	return true;
}

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI
