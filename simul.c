
#include <stdlib.h>

#include "cpu.h"
#include "systeme.h"


/* ============================================================================
 _   _ _____   ____   _    ____    __  __  ___  ____ ___ _____ ___ _____ ____  
| \ | | ____| |  _ \ / \  / ___|  |  \/  |/ _ \|  _ \_ _|  ___|_ _| ____|  _ \ 
|  \| |  _|   | |_) / _ \ \___ \  | |\/| | | | | | | | || |_   | ||  _| | |_) |
| |\  | |___  |  __/ ___ \ ___) | | |  | | |_| | |_| | ||  _|  | || |___|  _ < 
|_| \_|_____| |_| /_/   \_\____/  |_|  |_|\___/|____/___|_|   |___|_____|_| \_\
                                                                                                                                   
============================================================================ */

/**********************************************************
** fonction principale (à ne pas modifier !)
***********************************************************/

int main(void) {
	//init-cpu();
    for(PSW mep = system_init();;) {
        mep = cpu(mep);
        mep = process_interrupt(mep);
    }
    return (EXIT_SUCCESS);
}

