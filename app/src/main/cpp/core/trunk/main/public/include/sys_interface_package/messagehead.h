/**
 *@file MessageHead.h
 *@brief Definition of MessageHead
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_MESSAGE_HEAD_H          //NOLINT
#define CHP_MESSAGE_HEAD_H          //NOLINT

#include "../../../public/include/sys_interface_package/common.h"
#include <boost/shared_ptr.hpp>

namespace ysos {

typedef struct MessageHead {
  // Defined in MessageType.h.
  uint32_t message_type;

  // Total length of the message, include this head.
  uint32_t message_length;

  // TimeStamp or SerialNumber.
  uint32_t timestamp;

  // Current packet index based 0 as high 16 bits, and total as low 16 bits.
  // MessageData follows it.
  uint32_t packet_index_total;
} MessageHead;

typedef boost::shared_ptr<MessageHead> MessageHeadPtr;
}   // namespace ysos

#define MAKE_PACKET_INDEX_TOTAL(index, total)   \
  ((((uint32_t)(index)) << 16) + (total))

//  这里以优先级数指示优先级.//NOLINT
//  优先级数越大, 优先级越高; 优先级越高, 优先级数越大//NOLINT
#define MESSAGE_PRIORITY_LEVEL_HIGH         3       //  高优先级级别//NOLINT
#define MESSAGE_PRIORITY_LEVEL_NORMAL       2       //  正常优先级级别//NOLINT
#define MESSAGE_PRIORITY_LEVEL_LOW          1       //  低优先级级别//NOLINT
#define MESSAGE_PRIORITY_LEVEL_IDLE         0       //  空闲优先级级别//NOLINT
#define MESSAGE_PRIORITY_ADJUST_INC_3       3       //  在优先级数提高3//NOLINT
#define MESSAGE_PRIORITY_ADJUST_INC_2       2       //  在优先级数提高2//NOLINT
#define MESSAGE_PRIORITY_ADJUST_INC_1       1       //  在优先级数提高1//NOLINT
#define MESSAGE_PRIORITY_ADJUST_NONE        0       //  在优先级数不变//NOLINT
#define MESSAGE_PRIORITY_ADJUST_DEC_1       -1      //  在优先级数降低1//NOLINT
#define MESSAGE_PRIORITY_ADJUST_DEC_2       -2      //  在优先级数降低2//NOLINT
#define MESSAGE_PRIORITY_ADJUST_DEC_3       -3      //  在优先级数降低3//NOLINT
#define MESSAGE_PRIORITY_ADJUST_DEC_4       -4      //  在优先级数降低4//NOLINT
#define MESSAGE_PRIORITY_DEFAULT_LEVEL      MESSAGE_PRIORITY_LEVEL_LOW
#define MESSAGE_PRIORITY_DEFAULT_ADJUST     MESSAGE_PRIORITY_ADJUST_NONE
#define MESSAGE_PRIORITY_DEFAULT            (MESSAGE_PRIORITY_DEFAULT_LEVEL * 8 + 4)

#define GET_MESSAGE_PRIORITY(LEVEL, ADJUST) ((LEVEL) * 8 + 4 + (ADJUST))

#endif  // CHP_MESSAGE_HEAD_H       //NOLINT
