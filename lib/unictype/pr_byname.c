/* Properties of Unicode characters.
   Copyright (C) 2007, 2011-2022 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2007.

   This file is free software.
   It is dual-licensed under "the GNU LGPLv3+ or the GNU GPLv2+".
   You can redistribute it and/or modify it under either
     - the terms of the GNU Lesser General Public License as published
       by the Free Software Foundation, either version 3, or (at your
       option) any later version, or
     - the terms of the GNU General Public License as published by the
       Free Software Foundation; either version 2, or (at your option)
       any later version, or
     - the same dual license "the GNU LGPLv3+ or the GNU GPLv2+".

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License and the GNU General Public License
   for more details.

   You should have received a copy of the GNU Lesser General Public
   License and of the GNU General Public License along with this
   program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "unictype.h"

#include <stdlib.h>
#include <string.h>

/* Indices stored in the 'struct named_category' elements of the perfect hash
   table.  We don't use uc_general_category_t values or their addresses
   directly, because this would introduce load-time relocations.  */
enum
{
  /* General.  */
  UC_PROPERTY_INDEX_WHITE_SPACE,
  UC_PROPERTY_INDEX_ALPHABETIC,
  UC_PROPERTY_INDEX_OTHER_ALPHABETIC,
  UC_PROPERTY_INDEX_NOT_A_CHARACTER,
  UC_PROPERTY_INDEX_DEFAULT_IGNORABLE_CODE_POINT,
  UC_PROPERTY_INDEX_OTHER_DEFAULT_IGNORABLE_CODE_POINT,
  UC_PROPERTY_INDEX_DEPRECATED,
  UC_PROPERTY_INDEX_LOGICAL_ORDER_EXCEPTION,
  UC_PROPERTY_INDEX_VARIATION_SELECTOR,
  UC_PROPERTY_INDEX_PRIVATE_USE,
  UC_PROPERTY_INDEX_UNASSIGNED_CODE_VALUE,
  /* Case.  */
  UC_PROPERTY_INDEX_UPPERCASE,
  UC_PROPERTY_INDEX_OTHER_UPPERCASE,
  UC_PROPERTY_INDEX_LOWERCASE,
  UC_PROPERTY_INDEX_OTHER_LOWERCASE,
  UC_PROPERTY_INDEX_TITLECASE,
  UC_PROPERTY_INDEX_CASED,
  UC_PROPERTY_INDEX_CASE_IGNORABLE,
  UC_PROPERTY_INDEX_CHANGES_WHEN_LOWERCASED,
  UC_PROPERTY_INDEX_CHANGES_WHEN_UPPERCASED,
  UC_PROPERTY_INDEX_CHANGES_WHEN_TITLECASED,
  UC_PROPERTY_INDEX_CHANGES_WHEN_CASEFOLDED,
  UC_PROPERTY_INDEX_CHANGES_WHEN_CASEMAPPED,
  UC_PROPERTY_INDEX_SOFT_DOTTED,
  /* Identifiers.  */
  UC_PROPERTY_INDEX_ID_START,
  UC_PROPERTY_INDEX_OTHER_ID_START,
  UC_PROPERTY_INDEX_ID_CONTINUE,
  UC_PROPERTY_INDEX_OTHER_ID_CONTINUE,
  UC_PROPERTY_INDEX_XID_START,
  UC_PROPERTY_INDEX_XID_CONTINUE,
  UC_PROPERTY_INDEX_PATTERN_WHITE_SPACE,
  UC_PROPERTY_INDEX_PATTERN_SYNTAX,
  /* Shaping and rendering.  */
  UC_PROPERTY_INDEX_JOIN_CONTROL,
  UC_PROPERTY_INDEX_GRAPHEME_BASE,
  UC_PROPERTY_INDEX_GRAPHEME_EXTEND,
  UC_PROPERTY_INDEX_OTHER_GRAPHEME_EXTEND,
  UC_PROPERTY_INDEX_GRAPHEME_LINK,
  /* Bidi.  */
  UC_PROPERTY_INDEX_BIDI_CONTROL,
  UC_PROPERTY_INDEX_BIDI_LEFT_TO_RIGHT,
  UC_PROPERTY_INDEX_BIDI_HEBREW_RIGHT_TO_LEFT,
  UC_PROPERTY_INDEX_BIDI_ARABIC_RIGHT_TO_LEFT,
  UC_PROPERTY_INDEX_BIDI_EUROPEAN_DIGIT,
  UC_PROPERTY_INDEX_BIDI_EUR_NUM_SEPARATOR,
  UC_PROPERTY_INDEX_BIDI_EUR_NUM_TERMINATOR,
  UC_PROPERTY_INDEX_BIDI_ARABIC_DIGIT,
  UC_PROPERTY_INDEX_BIDI_COMMON_SEPARATOR,
  UC_PROPERTY_INDEX_BIDI_BLOCK_SEPARATOR,
  UC_PROPERTY_INDEX_BIDI_SEGMENT_SEPARATOR,
  UC_PROPERTY_INDEX_BIDI_WHITESPACE,
  UC_PROPERTY_INDEX_BIDI_NON_SPACING_MARK,
  UC_PROPERTY_INDEX_BIDI_BOUNDARY_NEUTRAL,
  UC_PROPERTY_INDEX_BIDI_PDF,
  UC_PROPERTY_INDEX_BIDI_EMBEDDING_OR_OVERRIDE,
  UC_PROPERTY_INDEX_BIDI_OTHER_NEUTRAL,
  /* Numeric.  */
  UC_PROPERTY_INDEX_HEX_DIGIT,
  UC_PROPERTY_INDEX_ASCII_HEX_DIGIT,
  /* CJK.  */
  UC_PROPERTY_INDEX_IDEOGRAPHIC,
  UC_PROPERTY_INDEX_UNIFIED_IDEOGRAPH,
  UC_PROPERTY_INDEX_RADICAL,
  UC_PROPERTY_INDEX_IDS_BINARY_OPERATOR,
  UC_PROPERTY_INDEX_IDS_TRINARY_OPERATOR,
  /* Misc.  */
  UC_PROPERTY_INDEX_ZERO_WIDTH,
  UC_PROPERTY_INDEX_SPACE,
  UC_PROPERTY_INDEX_NON_BREAK,
  UC_PROPERTY_INDEX_ISO_CONTROL,
  UC_PROPERTY_INDEX_FORMAT_CONTROL,
  UC_PROPERTY_INDEX_DASH,
  UC_PROPERTY_INDEX_HYPHEN,
  UC_PROPERTY_INDEX_PUNCTUATION,
  UC_PROPERTY_INDEX_LINE_SEPARATOR,
  UC_PROPERTY_INDEX_PARAGRAPH_SEPARATOR,
  UC_PROPERTY_INDEX_QUOTATION_MARK,
  UC_PROPERTY_INDEX_SENTENCE_TERMINAL,
  UC_PROPERTY_INDEX_TERMINAL_PUNCTUATION,
  UC_PROPERTY_INDEX_CURRENCY_SYMBOL,
  UC_PROPERTY_INDEX_MATH,
  UC_PROPERTY_INDEX_OTHER_MATH,
  UC_PROPERTY_INDEX_PAIRED_PUNCTUATION,
  UC_PROPERTY_INDEX_LEFT_OF_PAIR,
  UC_PROPERTY_INDEX_COMBINING,
  UC_PROPERTY_INDEX_COMPOSITE,
  UC_PROPERTY_INDEX_DECIMAL_DIGIT,
  UC_PROPERTY_INDEX_NUMERIC,
  UC_PROPERTY_INDEX_DIACRITIC,
  UC_PROPERTY_INDEX_EXTENDER,
  UC_PROPERTY_INDEX_IGNORABLE_CONTROL
};

/* Get gperf generated lookup function.  */
#include "unictype/pr_byname.h"

static const uc_property_t UC_PROPERTY_NONE = { NULL };

uc_property_t
uc_property_byname (const char *property_name)
{
  char buf[MAX_WORD_LENGTH + 1];
  const char *cp;
  char *bp;
  unsigned int count;
  const struct named_property *found;

  for (cp = property_name, bp = buf, count = MAX_WORD_LENGTH + 1; ; cp++, bp++)
    {
      unsigned char c = (unsigned char) *cp;
      if (c >= 0x80)
        goto invalid;
      if (c >= 'A' && c <= 'Z')
        c += 'a' - 'A';
      else if (c == ' ' || c == '-')
        c = '_';
      *bp = c;
      if (c == '\0')
        break;
      if (--count == 0)
        goto invalid;
    }
  found = uc_property_lookup (buf, bp - buf);
  if (found != NULL)
    /* Use a 'switch' statement here, because a table would introduce load-time
       relocations.  */
    switch (found->property_index)
      {
      case UC_PROPERTY_INDEX_WHITE_SPACE:
        return UC_PROPERTY_WHITE_SPACE;
      case UC_PROPERTY_INDEX_ALPHABETIC:
        return UC_PROPERTY_ALPHABETIC;
      case UC_PROPERTY_INDEX_OTHER_ALPHABETIC:
        return UC_PROPERTY_OTHER_ALPHABETIC;
      case UC_PROPERTY_INDEX_NOT_A_CHARACTER:
        return UC_PROPERTY_NOT_A_CHARACTER;
      case UC_PROPERTY_INDEX_DEFAULT_IGNORABLE_CODE_POINT:
        return UC_PROPERTY_DEFAULT_IGNORABLE_CODE_POINT;
      case UC_PROPERTY_INDEX_OTHER_DEFAULT_IGNORABLE_CODE_POINT:
        return UC_PROPERTY_OTHER_DEFAULT_IGNORABLE_CODE_POINT;
      case UC_PROPERTY_INDEX_DEPRECATED:
        return UC_PROPERTY_DEPRECATED;
      case UC_PROPERTY_INDEX_LOGICAL_ORDER_EXCEPTION:
        return UC_PROPERTY_LOGICAL_ORDER_EXCEPTION;
      case UC_PROPERTY_INDEX_VARIATION_SELECTOR:
        return UC_PROPERTY_VARIATION_SELECTOR;
      case UC_PROPERTY_INDEX_PRIVATE_USE:
        return UC_PROPERTY_PRIVATE_USE;
      case UC_PROPERTY_INDEX_UNASSIGNED_CODE_VALUE:
        return UC_PROPERTY_UNASSIGNED_CODE_VALUE;
      case UC_PROPERTY_INDEX_UPPERCASE:
        return UC_PROPERTY_UPPERCASE;
      case UC_PROPERTY_INDEX_OTHER_UPPERCASE:
        return UC_PROPERTY_OTHER_UPPERCASE;
      case UC_PROPERTY_INDEX_LOWERCASE:
        return UC_PROPERTY_LOWERCASE;
      case UC_PROPERTY_INDEX_OTHER_LOWERCASE:
        return UC_PROPERTY_OTHER_LOWERCASE;
      case UC_PROPERTY_INDEX_TITLECASE:
        return UC_PROPERTY_TITLECASE;
      case UC_PROPERTY_INDEX_CASED:
        return UC_PROPERTY_CASED;
      case UC_PROPERTY_INDEX_CASE_IGNORABLE:
        return UC_PROPERTY_CASE_IGNORABLE;
      case UC_PROPERTY_INDEX_CHANGES_WHEN_LOWERCASED:
        return UC_PROPERTY_CHANGES_WHEN_LOWERCASED;
      case UC_PROPERTY_INDEX_CHANGES_WHEN_UPPERCASED:
        return UC_PROPERTY_CHANGES_WHEN_UPPERCASED;
      case UC_PROPERTY_INDEX_CHANGES_WHEN_TITLECASED:
        return UC_PROPERTY_CHANGES_WHEN_TITLECASED;
      case UC_PROPERTY_INDEX_CHANGES_WHEN_CASEFOLDED:
        return UC_PROPERTY_CHANGES_WHEN_CASEFOLDED;
      case UC_PROPERTY_INDEX_CHANGES_WHEN_CASEMAPPED:
        return UC_PROPERTY_CHANGES_WHEN_CASEMAPPED;
      case UC_PROPERTY_INDEX_SOFT_DOTTED:
        return UC_PROPERTY_SOFT_DOTTED;
      case UC_PROPERTY_INDEX_ID_START:
        return UC_PROPERTY_ID_START;
      case UC_PROPERTY_INDEX_OTHER_ID_START:
        return UC_PROPERTY_OTHER_ID_START;
      case UC_PROPERTY_INDEX_ID_CONTINUE:
        return UC_PROPERTY_ID_CONTINUE;
      case UC_PROPERTY_INDEX_OTHER_ID_CONTINUE:
        return UC_PROPERTY_OTHER_ID_CONTINUE;
      case UC_PROPERTY_INDEX_XID_START:
        return UC_PROPERTY_XID_START;
      case UC_PROPERTY_INDEX_XID_CONTINUE:
        return UC_PROPERTY_XID_CONTINUE;
      case UC_PROPERTY_INDEX_PATTERN_WHITE_SPACE:
        return UC_PROPERTY_PATTERN_WHITE_SPACE;
      case UC_PROPERTY_INDEX_PATTERN_SYNTAX:
        return UC_PROPERTY_PATTERN_SYNTAX;
      case UC_PROPERTY_INDEX_JOIN_CONTROL:
        return UC_PROPERTY_JOIN_CONTROL;
      case UC_PROPERTY_INDEX_GRAPHEME_BASE:
        return UC_PROPERTY_GRAPHEME_BASE;
      case UC_PROPERTY_INDEX_GRAPHEME_EXTEND:
        return UC_PROPERTY_GRAPHEME_EXTEND;
      case UC_PROPERTY_INDEX_OTHER_GRAPHEME_EXTEND:
        return UC_PROPERTY_OTHER_GRAPHEME_EXTEND;
      case UC_PROPERTY_INDEX_GRAPHEME_LINK:
        return UC_PROPERTY_GRAPHEME_LINK;
      case UC_PROPERTY_INDEX_BIDI_CONTROL:
        return UC_PROPERTY_BIDI_CONTROL;
      case UC_PROPERTY_INDEX_BIDI_LEFT_TO_RIGHT:
        return UC_PROPERTY_BIDI_LEFT_TO_RIGHT;
      case UC_PROPERTY_INDEX_BIDI_HEBREW_RIGHT_TO_LEFT:
        return UC_PROPERTY_BIDI_HEBREW_RIGHT_TO_LEFT;
      case UC_PROPERTY_INDEX_BIDI_ARABIC_RIGHT_TO_LEFT:
        return UC_PROPERTY_BIDI_ARABIC_RIGHT_TO_LEFT;
      case UC_PROPERTY_INDEX_BIDI_EUROPEAN_DIGIT:
        return UC_PROPERTY_BIDI_EUROPEAN_DIGIT;
      case UC_PROPERTY_INDEX_BIDI_EUR_NUM_SEPARATOR:
        return UC_PROPERTY_BIDI_EUR_NUM_SEPARATOR;
      case UC_PROPERTY_INDEX_BIDI_EUR_NUM_TERMINATOR:
        return UC_PROPERTY_BIDI_EUR_NUM_TERMINATOR;
      case UC_PROPERTY_INDEX_BIDI_ARABIC_DIGIT:
        return UC_PROPERTY_BIDI_ARABIC_DIGIT;
      case UC_PROPERTY_INDEX_BIDI_COMMON_SEPARATOR:
        return UC_PROPERTY_BIDI_COMMON_SEPARATOR;
      case UC_PROPERTY_INDEX_BIDI_BLOCK_SEPARATOR:
        return UC_PROPERTY_BIDI_BLOCK_SEPARATOR;
      case UC_PROPERTY_INDEX_BIDI_SEGMENT_SEPARATOR:
        return UC_PROPERTY_BIDI_SEGMENT_SEPARATOR;
      case UC_PROPERTY_INDEX_BIDI_WHITESPACE:
        return UC_PROPERTY_BIDI_WHITESPACE;
      case UC_PROPERTY_INDEX_BIDI_NON_SPACING_MARK:
        return UC_PROPERTY_BIDI_NON_SPACING_MARK;
      case UC_PROPERTY_INDEX_BIDI_BOUNDARY_NEUTRAL:
        return UC_PROPERTY_BIDI_BOUNDARY_NEUTRAL;
      case UC_PROPERTY_INDEX_BIDI_PDF:
        return UC_PROPERTY_BIDI_PDF;
      case UC_PROPERTY_INDEX_BIDI_EMBEDDING_OR_OVERRIDE:
        return UC_PROPERTY_BIDI_EMBEDDING_OR_OVERRIDE;
      case UC_PROPERTY_INDEX_BIDI_OTHER_NEUTRAL:
        return UC_PROPERTY_BIDI_OTHER_NEUTRAL;
      case UC_PROPERTY_INDEX_HEX_DIGIT:
        return UC_PROPERTY_HEX_DIGIT;
      case UC_PROPERTY_INDEX_ASCII_HEX_DIGIT:
        return UC_PROPERTY_ASCII_HEX_DIGIT;
      case UC_PROPERTY_INDEX_IDEOGRAPHIC:
        return UC_PROPERTY_IDEOGRAPHIC;
      case UC_PROPERTY_INDEX_UNIFIED_IDEOGRAPH:
        return UC_PROPERTY_UNIFIED_IDEOGRAPH;
      case UC_PROPERTY_INDEX_RADICAL:
        return UC_PROPERTY_RADICAL;
      case UC_PROPERTY_INDEX_IDS_BINARY_OPERATOR:
        return UC_PROPERTY_IDS_BINARY_OPERATOR;
      case UC_PROPERTY_INDEX_IDS_TRINARY_OPERATOR:
        return UC_PROPERTY_IDS_TRINARY_OPERATOR;
      case UC_PROPERTY_INDEX_ZERO_WIDTH:
        return UC_PROPERTY_ZERO_WIDTH;
      case UC_PROPERTY_INDEX_SPACE:
        return UC_PROPERTY_SPACE;
      case UC_PROPERTY_INDEX_NON_BREAK:
        return UC_PROPERTY_NON_BREAK;
      case UC_PROPERTY_INDEX_ISO_CONTROL:
        return UC_PROPERTY_ISO_CONTROL;
      case UC_PROPERTY_INDEX_FORMAT_CONTROL:
        return UC_PROPERTY_FORMAT_CONTROL;
      case UC_PROPERTY_INDEX_DASH:
        return UC_PROPERTY_DASH;
      case UC_PROPERTY_INDEX_HYPHEN:
        return UC_PROPERTY_HYPHEN;
      case UC_PROPERTY_INDEX_PUNCTUATION:
        return UC_PROPERTY_PUNCTUATION;
      case UC_PROPERTY_INDEX_LINE_SEPARATOR:
        return UC_PROPERTY_LINE_SEPARATOR;
      case UC_PROPERTY_INDEX_PARAGRAPH_SEPARATOR:
        return UC_PROPERTY_PARAGRAPH_SEPARATOR;
      case UC_PROPERTY_INDEX_QUOTATION_MARK:
        return UC_PROPERTY_QUOTATION_MARK;
      case UC_PROPERTY_INDEX_SENTENCE_TERMINAL:
        return UC_PROPERTY_SENTENCE_TERMINAL;
      case UC_PROPERTY_INDEX_TERMINAL_PUNCTUATION:
        return UC_PROPERTY_TERMINAL_PUNCTUATION;
      case UC_PROPERTY_INDEX_CURRENCY_SYMBOL:
        return UC_PROPERTY_CURRENCY_SYMBOL;
      case UC_PROPERTY_INDEX_MATH:
        return UC_PROPERTY_MATH;
      case UC_PROPERTY_INDEX_OTHER_MATH:
        return UC_PROPERTY_OTHER_MATH;
      case UC_PROPERTY_INDEX_PAIRED_PUNCTUATION:
        return UC_PROPERTY_PAIRED_PUNCTUATION;
      case UC_PROPERTY_INDEX_LEFT_OF_PAIR:
        return UC_PROPERTY_LEFT_OF_PAIR;
      case UC_PROPERTY_INDEX_COMBINING:
        return UC_PROPERTY_COMBINING;
      case UC_PROPERTY_INDEX_COMPOSITE:
        return UC_PROPERTY_COMPOSITE;
      case UC_PROPERTY_INDEX_DECIMAL_DIGIT:
        return UC_PROPERTY_DECIMAL_DIGIT;
      case UC_PROPERTY_INDEX_NUMERIC:
        return UC_PROPERTY_NUMERIC;
      case UC_PROPERTY_INDEX_DIACRITIC:
        return UC_PROPERTY_DIACRITIC;
      case UC_PROPERTY_INDEX_EXTENDER:
        return UC_PROPERTY_EXTENDER;
      case UC_PROPERTY_INDEX_IGNORABLE_CONTROL:
        return UC_PROPERTY_IGNORABLE_CONTROL;
      default:
        abort ();
      }
 invalid:
  return UC_PROPERTY_NONE;
}
