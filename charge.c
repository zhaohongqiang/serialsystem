#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include "charge.h"
#include "tcu.h"
#include "log.h"
#include "error.h"

struct charge_task tom;

// 充电任务结构
struct charge_task *task = &tom;
