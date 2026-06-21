#include "Navigation.h"

// =======================================================
// SET CỨNG TEXT CHỈ ĐƯỜNG (HARDCODED NAVIGATION)
// =======================================================
const char *lay_huong_dan(int start, int dest)
{
    // TỪ: TUYỂN SINH (1) đi các phòng khác
    if (start == 1 && dest == 2)
        return "Đi thẳng 20m, rẽ trái vào IT";
    if (start == 1 && dest == 3)
        return "Đi thẳng 35m, rẽ trái Thư Viện";
    if (start == 1 && dest == 4)
        return "Đi thẳng 10m, rẽ phải tới WC";

    // TỪ: PHÒNG IT (2) đi các phòng khác
    if (start == 2 && dest == 1)
        return "Rẽ phải, đi thẳng 20m";
    if (start == 2 && dest == 3)
        return "Rẽ trái, đi thẳng 15m";
    if (start == 2 && dest == 4)
        return "Rẽ phải, đi 20m rồi rẽ trái";

    // TỪ: THƯ VIỆN (3) đi các phòng khác
    if (start == 3 && dest == 1)
        return "Rẽ phải, đi thẳng 35m";
    if (start == 3 && dest == 2)
        return "Rẽ phải, đi thẳng 15m";
    if (start == 3 && dest == 4)
        return "Quay lại hành lang, đi 30m";

    // TỪ: NHÀ WC (4) đi các phòng khác
    if (start == 4 && dest == 1)
        return "Rẽ trái, đi thẳng 10m";
    if (start == 4 && dest == 2)
        return "Rẽ phải đi 20m, rẽ trái";
    if (start == 4 && dest == 3)
        return "Đi thẳng 30m tới cuối đường";

    return "Đang tính toán chỉ đường...";
}