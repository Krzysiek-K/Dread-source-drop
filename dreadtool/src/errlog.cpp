
#include "common.h"


ErrorLog elog;

MenuService elog_ms;


void ErrorLog::Clear()
{
	for( int i=0; i<(int)log.size(); i++ )
		delete log[i];
	log.clear();
}


void ErrorLog::Message(ToolObject *obj, uint32_t color, error_level_t lvl, const string &msg)
{
	LogEntry *e = new LogEntry();
	e->object = obj;
	e->level = lvl;
	e->has_object = (obj!=NULL);
	e->color = color;
	e->message = msg;
	log.push_back(e);
}

ErrorLog::LogEntry *ErrorLog::MapMessage(ToolObject *obj, uint32_t color, error_level_t lvl, const ivec2 &coord, const string &msg)
{
	LogEntry *e = new LogEntry();
	e->object = obj;
	e->level = lvl;
	e->has_object = (obj!=NULL);
	e->color = color;
	e->message = msg;
	e->has_coord = true;
	e->coord = coord;
	log.push_back(e);
	return e;
}

ErrorLog::error_level_t ErrorLog::GetErrorLevel()
{
	error_level_t maxlevel = LEVEL_NONE;

	for( int i=0; i<(int)log.size(); i++ )
	{
		LogEntry &e = *log[i];
		if( !e.has_object || e.object.get() )
		{
			if( e.level > maxlevel )
				maxlevel = e.level;
		}
	}

	return maxlevel;
}

void ErrorLog::RemoveMessagesOf(ToolObject *obj)
{
	for( int i=0; i<(int)log.size(); i++ )
	{
		LogEntry &e = *log[i];
		if( e.object==obj )
		{
			// force GC to remove it
			e.object = NULL;
			e.has_object = true;
		}
	}
}


void ErrorLog::Draw(int x1, int x2)
{
	elog_ms.Begin(x1, 0, x2, Dev.GetSizeY());
	elog_ms.Panel();
	elog_ms.Label("Error log");

	elog_ms.ListStart();
	elog_ms.ListItem("", 0xFFC0C0C0, false);
	int dest = 0;
	for( int i=0; i<(int)log.size(); i++ )
	{
		LogEntry &e = *log[i];
		if( e.object.get() || !e.has_object)
		{
			//if( elog_ms.ListItem(format("%08X %s", e.object.get(), e.message.c_str()).c_str(), e.color, false) )
			if( elog_ms.ListItem(e.message.c_str(), e.color, false) )
			{
				if( e.object.get() )
					e.object->Focus();
			}
			log[dest++] = log[i];
		}
		else
			delete log[i];
	}
	log.resize(dest);

	if( GetErrorLevel() <= LEVEL_INFO )
		elog_ms.ListItem("All OK.", 0xFFC0C0C0, false);

	elog_ms.ListItem("", 0xFFC0C0C0, false);
	elog_ms.ListItem("", 0xFFC0C0C0, false);
	elog_ms.ListEnd();


	elog_ms.EndPanel();
	elog_ms.End();
}
