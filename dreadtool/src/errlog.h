
#pragma once

class ToolObject : public Collectable {
public:
	virtual ~ToolObject() {}

	virtual void Focus() {}
};


class ErrorLog {
public:
	enum error_level_t
	{
		LEVEL_NONE		= 0,
		LEVEL_INFO,
		LEVEL_WARNING,
		LEVEL_ERROR,
	};

	struct MapDebugLine {
		vec2	v1, v2;
		DWORD	color = 0xFF00FF;

		MapDebugLine(const vec2 &_v1, const vec2 &_v2, DWORD _color) : v1(_v1), v2(_v2), color(_color) {}
	};

	class LogEntry {
	public:
		weak_ptr<ToolObject>	object;
		error_level_t			level = LEVEL_NONE;
		bool					has_object = false;
		string					message;
		uint32_t				color = 0xFFFF0000;
		bool					has_coord = false;
		ivec2					coord;
		vector<MapDebugLine>	map_debug;
	};

	vector<LogEntry*>	log;

	~ErrorLog() { Clear(); }
	
	void Clear();


	void Message(ToolObject *obj, uint32_t color, error_level_t lvl, const string &msg);
	LogEntry *MapMessage(ToolObject *obj, uint32_t color, error_level_t lvl, const ivec2 &coord, const string &msg);

	void Error(ToolObject *obj, const string &msg) {
		Message(obj, 0xFFFF0000, LEVEL_ERROR, msg);
		printf("ERROR: %s\n", msg.c_str());
	}
	void Warn(ToolObject *obj, const string &msg) {
		Message(obj, 0xFFFFFF00, LEVEL_WARNING, msg);
		printf("Warning: %s\n", msg.c_str());
	}
	void Info(ToolObject *obj, const string &msg) {
		Message(obj, 0xFFC0C0C0, LEVEL_INFO, msg);
	}

	LogEntry *MapError(ToolObject *obj, const ivec2 &coord, const string &msg)
	{
		printf("ERROR (%d,%d): %s\n", coord.x, coord.y, msg.c_str());
		return MapMessage(obj, 0xFFFF0000, LEVEL_ERROR, coord, msg);
	}
	LogEntry *MapWarn(ToolObject *obj, const ivec2 &coord, const string &msg)
	{
		printf("Warning (%d,%d): %s\n", coord.x, coord.y, msg.c_str());
		return MapMessage(obj, 0xFFFFFF00, LEVEL_WARNING, coord, msg);
	}

	error_level_t GetErrorLevel();
	void RemoveMessagesOf(ToolObject *obj);


	void Draw(int x1, int x2);
};


extern ErrorLog elog;
