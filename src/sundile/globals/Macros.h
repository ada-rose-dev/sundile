
#ifndef SUNDILE_MACROS
#define SUNDILE_MACROS

//-- Component macros
struct Component {};

#define SERIALIZE(_componentName, ...) template <class Archive> void serialize(Archive& archive) { archive(__VA_ARGS__);}

#define COMPONENT(_componentName) \
	namespace sundile { namespace Components { struct _componentName:Component { static constexpr char const* __name = #_componentName;
#define END_COMPONENT };}};

//-- System macros
#define SYSTEM(name) namespace sundile { using namespace Components; namespace Systems { namespace name {
#define END_SYSTEM }}}

//-- OS macros
#ifdef _MSC_VER
#define SUNDILE_DEBUG_BREAK assert(0);//__debug_break();
#else
#define SUNDILE_DEBUG_BREAK assert(0);
#endif

//-- GL macros
#define SUNDILE_GLSL_VERSION "#version 330 core"

//-- Versioning macros
#define SUNDILE_VERSION_NUMBER 0.01
#define PROJECT_EXTENSION ".sunproj"

#endif