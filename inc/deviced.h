/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LOCKSCREEN_DEVICED_H_
#define _LOCKSCREEN_DEVICED_H_

#include <stdbool.h>

/**
 * @addtogroup Models
 * @{
 */

/**
 * @defgroup Deviced Deviced
 */

/**
 * @addtogroup Deviced
 * @{
 */

/**
 * @brief Inform deviced deamon that lockscreen goes into background.
 *
 * @note This is temporary solution until some sane method to
 * handle camera-app requirement will be found.
 *
 * @return 0 when state was successfully changed, other value on failure.
 */
int lockscreen_deviced_lockscreen_background_state_set(bool val);
/**
 * @}
 */

/**
 * @}
 */

#endif