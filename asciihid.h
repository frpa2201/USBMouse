/* asciihid.h */
/* ASCII to HID Keyboard lookup table */
/* Copyright (c) Phil Wright 2008 */

#ifndef HIDTABLE_H
#define HIDTABLE_H

/* Default is UK keyboard layout */
/* #define US_KEYBOARD */

/* Modifiers */
#define SHIFT (1<<1)

typedef struct {
    unsigned char usage;
    unsigned char modifier;
} KEYMAP;

#ifdef US_KEYBOARD
/* US keyboard (as HID standard) */
#define KEYMAP_SIZE (128)
const KEYMAP keymap[KEYMAP_SIZE] = {
{0, 0},             /* NUL */
{0, 0},             /* SOH */
{0, 0},             /* STX */
{0, 0},             /* ETX */
{0, 0},             /* EOT */
{0, 0},             /* ENQ */
{0, 0},             /* ACK */  
{0, 0},             /* BEL */
{0x2a, 0},          /* BS  */  /* Keyboard Delete (Backspace) */ 
{0x2b, 0},          /* TAB */  /* Keyboard Tab */
{0x28, 0},          /* LF  */  /* Keyboard Return (Enter) */
{0, 0},             /* VT  */
{0, 0},             /* FF  */
{0, 0},             /* CR  */
{0, 0},             /* SO  */
{0, 0},             /* SI  */
{0, 0},             /* DEL */
{0, 0},             /* DC1 */
{0, 0},             /* DC2 */
{0, 0},             /* DC3 */
{0, 0},             /* DC4 */
{0, 0},             /* NAK */
{0, 0},             /* SYN */
{0, 0},             /* ETB */
{0, 0},             /* CAN */
{0, 0},             /* EM  */
{0, 0},             /* SUB */
{0, 0},             /* ESC */
{0, 0},             /* FS  */
{0, 0},             /* GS  */
{0, 0},             /* RS  */
{0, 0},             /* US  */
{0x2c, 0},          /*   */
{0x1e, SHIFT},      /* ! */
{0x34, SHIFT},      /* " */
{0x20, SHIFT},      /* # */
{0x21, SHIFT},      /* $ */
{0x22, SHIFT},      /* % */
{0x24, SHIFT},      /* & */
{0x34, 0},          /* ' */
{0x26, SHIFT},      /* ( */
{0x27, SHIFT},      /* ) */
{0x25, SHIFT},      /* * */
{0x2e, SHIFT},      /* + */
{0x36, 0},          /* , */
{0x2d, 0},          /* - */
{0x37, 0},          /* . */
{0x38, 0},          /* / */
{0x27, 0},          /* 0 */
{0x1e, 0},          /* 1 */
{0x1f, 0},          /* 2 */
{0x20, 0},          /* 3 */
{0x21, 0},          /* 4 */
{0x22, 0},          /* 5 */
{0x23, 0},          /* 6 */
{0x24, 0},          /* 7 */
{0x25, 0},          /* 8 */
{0x26, 0},          /* 9 */
{0x33, SHIFT},      /* : */
{0x33, 0},          /* ; */
{0x36, SHIFT},      /* < */
{0x2e, 0},          /* = */
{0x37, SHIFT},      /* > */
{0x38, SHIFT},      /* ? */
{0x1f, SHIFT},      /* @ */
{0x04, SHIFT},      /* A */
{0x05, SHIFT},      /* B */
{0x06, SHIFT},      /* C */
{0x07, SHIFT},      /* D */
{0x08, SHIFT},      /* E */
{0x09, SHIFT},      /* F */
{0x0a, SHIFT},      /* G */
{0x0b, SHIFT},      /* H */
{0x0c, SHIFT},      /* I */
{0x0d, SHIFT},      /* J */
{0x0e, SHIFT},      /* K */
{0x0f, SHIFT},      /* L */
{0x10, SHIFT},      /* M */
{0x11, SHIFT},      /* N */
{0x12, SHIFT},      /* O */
{0x13, SHIFT},      /* P */
{0x14, SHIFT},      /* Q */
{0x15, SHIFT},      /* R */
{0x16, SHIFT},      /* S */
{0x17, SHIFT},      /* T */
{0x18, SHIFT},      /* U */
{0x19, SHIFT},      /* V */
{0x1a, SHIFT},      /* W */
{0x1b, SHIFT},      /* X */
{0x1c, SHIFT},      /* Y */
{0x1d, SHIFT},      /* Z */
{0x2f, 0},          /* [ */
{0x31, 0},          /* \ */
{0x30, 0},          /* ] */
{0x23, SHIFT},      /* ^ */
{0x2d, SHIFT},      /* _ */
{0x35, 0},          /* ` */
{0x04, 0},          /* a */
{0x05, 0},          /* b */
{0x06, 0},          /* c */
{0x07, 0},          /* d */
{0x08, 0},          /* e */
{0x09, 0},          /* f */
{0x0a, 0},          /* g */
{0x0b, 0},          /* h */
{0x0c, 0},          /* i */
{0x0d, 0},          /* j */
{0x0e, 0},          /* k */
{0x0f, 0},          /* l */
{0x10, 0},          /* m */
{0x11, 0},          /* n */
{0x12, 0},          /* o */
{0x13, 0},          /* p */
{0x14, 0},          /* q */
{0x15, 0},          /* r */
{0x16, 0},          /* s */
{0x17, 0},          /* t */
{0x18, 0},          /* u */
{0x19, 0},          /* v */
{0x1a, 0},          /* w */
{0x1b, 0},          /* x */
{0x1c, 0},          /* y */
{0x1d, 0},          /* z */
{0x2f, SHIFT},      /* { */
{0x31, SHIFT},      /* | */
{0x30, SHIFT},      /* } */
{0x35, SHIFT},      /* ~ */
{0,0},              /* DEL */
};

#else
/* UK keyboard */
#define KEYMAP_SIZE (128)
const KEYMAP keymap[KEYMAP_SIZE] = {
{0, 0},             /* NUL */
{0, 0},             /* SOH */
{0, 0},             /* STX */
{0, 0},             /* ETX */
{0, 0},             /* EOT */
{0, 0},             /* ENQ */
{0, 0},             /* ACK */  
{0, 0},             /* BEL */
{0x2a, 0},          /* BS  */  /* Keyboard Delete (Backspace) */ 
{0x2b, 0},          /* TAB */  /* Keyboard Tab */
{0x28, 0},          /* LF  */  /* Keyboard Return (Enter) */
{0, 0},             /* VT  */
{0, 0},             /* FF  */
{0, 0},             /* CR  */
{0, 0},             /* SO  */
{0, 0},             /* SI  */
{0, 0},             /* DEL */
{0, 0},             /* DC1 */
{0, 0},             /* DC2 */
{0, 0},             /* DC3 */
{0, 0},             /* DC4 */
{0, 0},             /* NAK */
{0, 0},             /* SYN */
{0, 0},             /* ETB */
{0, 0},             /* CAN */
{0, 0},             /* EM  */
{0, 0},             /* SUB */
{0, 0},             /* ESC */
{0, 0},             /* FS  */
{0, 0},             /* GS  */
{0, 0},             /* RS  */
{0, 0},             /* US  */
{0x2c, 0},          /*   */
{0x1e, SHIFT},      /* ! */
{0x1f, SHIFT},      /* " */ 
{0x32, 0},          /* # */ 
{0x21, SHIFT},      /* $ */
{0x22, SHIFT},      /* % */
{0x24, SHIFT},      /* & */
{0x34, 0},          /* ' */
{0x26, SHIFT},      /* ( */
{0x27, SHIFT},      /* ) */
{0x25, SHIFT},      /* * */
{0x2e, SHIFT},      /* + */
{0x36, 0},          /* , */
{0x2d, 0},          /* - */
{0x37, 0},          /* . */
{0x38, 0},          /* / */
{0x27, 0},          /* 0 */
{0x1e, 0},          /* 1 */
{0x1f, 0},          /* 2 */
{0x20, 0},          /* 3 */
{0x21, 0},          /* 4 */
{0x22, 0},          /* 5 */
{0x23, 0},          /* 6 */
{0x24, 0},          /* 7 */
{0x25, 0},          /* 8 */
{0x26, 0},          /* 9 */
{0x33, SHIFT},      /* : */
{0x33, 0},          /* ; */
{0x36, SHIFT},      /* < */
{0x2e, 0},          /* = */
{0x37, SHIFT},      /* > */
{0x38, SHIFT},      /* ? */
{0x34, SHIFT},      /* @ */
{0x04, SHIFT},      /* A */
{0x05, SHIFT},      /* B */
{0x06, SHIFT},      /* C */
{0x07, SHIFT},      /* D */
{0x08, SHIFT},      /* E */
{0x09, SHIFT},      /* F */
{0x0a, SHIFT},      /* G */
{0x0b, SHIFT},      /* H */
{0x0c, SHIFT},      /* I */
{0x0d, SHIFT},      /* J */
{0x0e, SHIFT},      /* K */
{0x0f, SHIFT},      /* L */
{0x10, SHIFT},      /* M */
{0x11, SHIFT},      /* N */
{0x12, SHIFT},      /* O */
{0x13, SHIFT},      /* P */
{0x14, SHIFT},      /* Q */
{0x15, SHIFT},      /* R */
{0x16, SHIFT},      /* S */
{0x17, SHIFT},      /* T */
{0x18, SHIFT},      /* U */
{0x19, SHIFT},      /* V */
{0x1a, SHIFT},      /* W */
{0x1b, SHIFT},      /* X */
{0x1c, SHIFT},      /* Y */
{0x1d, SHIFT},      /* Z */
{0x2f, 0},          /* [ */
{0x64, 0},          /* \ */ 
{0x30, 0},          /* ] */
{0x23, SHIFT},      /* ^ */
{0x2d, SHIFT},      /* _ */
{0x35, 0},          /* ` */
{0x04, 0},          /* a */
{0x05, 0},          /* b */
{0x06, 0},          /* c */
{0x07, 0},          /* d */
{0x08, 0},          /* e */
{0x09, 0},          /* f */
{0x0a, 0},          /* g */
{0x0b, 0},          /* h */
{0x0c, 0},          /* i */
{0x0d, 0},          /* j */
{0x0e, 0},          /* k */
{0x0f, 0},          /* l */
{0x10, 0},          /* m */
{0x11, 0},          /* n */
{0x12, 0},          /* o */
{0x13, 0},          /* p */
{0x14, 0},          /* q */
{0x15, 0},          /* r */
{0x16, 0},          /* s */
{0x17, 0},          /* t */
{0x18, 0},          /* u */
{0x19, 0},          /* v */
{0x1a, 0},          /* w */
{0x1b, 0},          /* x */
{0x1c, 0},          /* y */
{0x1d, 0},          /* z */
{0x2f, SHIFT},      /* { */
{0x64, SHIFT},      /* | */ 
{0x30, SHIFT},      /* } */
{0x32, SHIFT},      /* ~ */ 
{0,0},             /* DEL */
};
#endif


#endif