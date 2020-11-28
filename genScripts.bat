
start "" "genScripts.exe路径" %cd%

::ENTITY_DEFS_PATH = "entity_defs路径"

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::使用方法::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

::1.替换该文档第一句中"可执行文件路径"到genScripts.exe的路径.例如"可执行文件路径"替换为"c:\genScripts.exe"
::2.第二句中ENTITY_DEFS_PATH更改为服务器asset下Entity_defs的路径。例如:"c:/kbengine/server_assets/scripts/entity_defs"
::3.该配置文件可以随意移动，所在目录即为生成文件所在目录。
::4.该行以下内容为生成代码模板,修改会直接反映到生成文件上。所有goto语句和//TAG注释行不可删改。
::5.直接执行该文件即可。

::----------------------------------------------------------------------------------------------------

goto LOGIC_EVENT_TEMP
#pragma once
#include "Engine/KBECommon.h"
#include "Engine/KBEvent.h"
#include "Engine/KBETypes.h"
#include "LogicEvent.generated.h"

//TAG-METHOD
UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_%%METHODNAME%% : public UKBEventData
{
	GENERATED_BODY()

public:
//TAG-ARG
	UPROPERTY(Category = KBEngine, BlueprintReadWrite, EditAnywhere)
	%%ARGTYPE%% %%ARGNAME%%;
//ARG-TAG
};

:LOGIC_EVENT_TEMP

::------------------------------------------------------------------------------------------------
::------------------------------------------------------------------------------------------------

goto ENTITY_HEAD_TEMP
#pragma once
#include "Engine/KBECommon.h"
#include "Engine/KBETypes.h"
#include "Engine/%%CLASSNAME%%Base.h"
#include "LogicEvent.h"

namespace KBEngine
{

class %%CLASSNAME%% : public %%CLASSNAME%%Base
{
public:
	%%CLASSNAME%%();
	virtual ~%%CLASSNAME%%();
public:
	virtual void __init__() override;
	virtual void onDestroy() override;
//TAG-BASE
	void %%METHODNAME%%(%%ARGDECL%%);
//BASE-TAG
//TAG-CELL
	void %%METHODNAME%%(%%ARGDECL%%);
//CELL-TAG
//TAG-CLIENT
	virtual void %%METHODNAME%%(%%ARGDECL%%) override;
//CLIENT-TAG
};

}
:ENTITY_HEAD_TEMP

::------------------------------------------------------------------------------------------------
::------------------------------------------------------------------------------------------------

goto ENTITY_CPP_TEMP

#include "%%CLASSNAME%%.h"
#include "Engine/Entity.h"
#include "Engine/KBEtypes.h"
#include "LogicEvent.h"

namespace KBEngine
{

%%CLASSNAME%%::%%CLASSNAME%%():
	%%CLASSNAME%%Base()
{
}

%%CLASSNAME%%::~%%CLASSNAME%%()
{
}

void %%CLASSNAME%%::__init__()
{
//TAG-REGEVENT
	KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC("%%CALLMETHOD%%", "%%CALLMETHOD%%", [this](const UKBEventData* pEventData)
	{
		const UKBEventData_%%CALLMETHOD%%& data = static_cast<const UKBEventData_%%CALLMETHOD%%&>(*pEventData);
		%%CALLMETHOD%%(%%DATALIST%%);
	});

//REGEVENT-TAG
}

void %%CLASSNAME%%::onDestroy()
{
	KBENGINE_DEREGISTER_ALL_EVENT();
}

//TAG-BASE
void %%CLASSNAME%%::%%METHODNAME%%(%%ARGDECL%%)
{
	DEBUG_MSG("%%CLASSNAME%%::%%METHODNAME%%(): %%ARGFORMAT%%",%%ARGLIST%%);
	pBaseEntityCall->%%METHODNAME%%(%%ARGLIST%%);
}

//BASE-TAG

//TAG-CELL
void %%CLASSNAME%%::%%METHODNAME%%(%%ARGDECL%%)
{
	DEBUG_MSG("%%CLASSNAME%%::%%METHODNAME%%(): %%ARGFORMAT%%",%%ARGLIST%%);
	pCellEntityCall->%%METHODNAME%%(%%ARGLIST%%);
}

//CELL-TAG

//TAG-CLIENT
void %%CLASSNAME%%::%%METHODNAME%%(%%ARGDECL%%)
{
	UKBEventData_%%METHODNAME%%* pEventData = NewObject<UKBEventData_%%METHODNAME%%>();
//TAG-DATAUP
	pEventData->%%ARGNAME%% = %%ARGNAME%%;
//DATAUP-TAG
	KBENGINE_EVENT_FIRE("%%METHODNAME%%", pEventData);
}

//CLIENT-TAG
}

:ENTITY_CPP_TEMP
