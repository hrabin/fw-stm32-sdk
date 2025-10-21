#ifndef CHSUM_H
#define CHSUM_H

#define	CHSUM32_START_VALUE (0x6b13ff7d)

u32 chsum32 (u8 *data, u32 len, u32 start_value);

#endif // ! CHSUM_H
