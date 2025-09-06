#include "../../kernel/src/lib/stdio.h"
#include "../../kernel/src/lib/string.h"

#include "re.h"             // tiny-regex-c header
#include "re_test.h"        // optional if you made a test header

void regex_test(void) {
    const char *pattern = "ab*c";   // matches "ac", "abc", "abbc", etc.
    const char *tests[] = {
        "ac",
        "abc",
        "abbc",
        "aXc",
        "hello world",
        NULL
    };

    printf("\n[Regex Test] Pattern: \"%s\"\n", pattern);

    for (int i = 0; tests[i] != NULL; i++) {
        int match_len = 0;

        // Simpler: use re_match() directly
        int result = re_match(pattern, tests[i], &match_len);

        if (result != -1) {
            printf("  Matched: \"%s\" (match length: %d)\n", tests[i], match_len);
        } else {
            printf("  Not matched: \"%s\"\n", tests[i]);
        }
    }
}
