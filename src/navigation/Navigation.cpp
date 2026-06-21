#include "Navigation.h"

// =======================================================
// SET CỨNG TEXT CHỈ ĐƯỜNG (HARDCODED NAVIGATION)
// =======================================================
const char *lay_huong_dan(int start, int dest)
{
    // TỪ: Cửa số (1) đi các phòng khác
    if (start == 1 && dest == 2)
        return "Đi thẳng 2m, rẽ phải, điểm đến phòng IT nằm bên phải";
    if (start == 1 && dest == 3)
        return "Đi thẳng 2mm, chếch phải, điểm đến thư viện nằm bên phải";
    if (start == 1 && dest == 4)
        return "Đi thẳng 10m, điểm đến nhà WC nằm phía trước";

    // TỪ: PHÒNG IT (2) đi các phòng khác
    if (start == 2 && dest == 1)
        return "Rẽ trái, đi thẳng 5m, điểm đến cửa số 1 nằm bên trái";
    if (start == 2 && dest == 3)
        return "Đi thẳng 10m, điểm đến thư viện nằm bên phải";
    if (start == 2 && dest == 4)
        return "Chếch trái, đi 20m, điểm đến nằm phía trước";

    // TỪ: THƯ VIỆN (3) đi các phòng khác
    if (start == 3 && dest == 1)
        return "đi thẳng 5m, rẽ trái, điểm đến cửa số 1 nằm bên trái";
    if (start == 3 && dest == 2)
        return "Đi thẳng 1m, rẽ trái,điểm đến phòng IT nằm phía trước";
    if (start == 3 && dest == 4)
        return "đi thẳng 5m, rẽ phải, điểm đến nhà WC nằm bên phải";

    // TỪ: NHÀ WC (4) đi các phòng khác
    if (start == 4 && dest == 1)
        return "Đi thẳng, điểm đến cửa số 1 nằm phía trước";
    if (start == 4 && dest == 2)
        return "Đi thẳng 5m, chếch trái, điểm đến phòng IT nằm bên trái";
    if (start == 4 && dest == 3)
        return "Đi thẳng 5m, rẽ trái, điểm đến thư viện nằm phía trước";

    return "Đang tính toán chỉ đường...";
}