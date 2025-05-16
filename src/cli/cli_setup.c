// STD LIBS
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// STM files
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

// Own CLI headers
#include "cli_setup.h"
#include "cli.h"

// Expand cli implementation here (must be in one file only)
#define EMBEDDED_CLI_IMPL

#include "embedded_cli.h"

// CLI buffer
static EmbeddedCli *cli;
static CLI_UINT cliBuffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];

// Bool to disable the interrupts, if CLI is not yet ready.
static bool cliIsReady = false;

// Write function used in 'setupCli()' to route the chars over UART.
static void writeCharToCli(EmbeddedCli *embeddedCli, char c) {
    uint8_t c_to_send = c;
    CDC_Transmit_FS(&c_to_send, 1);
}

// Function to setup the configuration settings for the CLI, based on the definitions from this header file
void setupCli() {
    // Initialize the CLI configuration settings
    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->cliBuffer = cliBuffer;
    config->cliBufferSize = CLI_BUFFER_SIZE;
    config->rxBufferSize = CLI_RX_BUFFER_SIZE;
    config->cmdBufferSize = CLI_CMD_BUFFER_SIZE;
    config->historyBufferSize = CLI_HISTORY_SIZE;
    config->maxBindingCount = CLI_MAX_BINDING_COUNT;

    // Create new CLI instance
    cli = embeddedCliNew(config);
    // Assign character write function
    cli->writeChar = writeCharToCli;

    // CLI init failed. Is there not enough memory allocated to the CLI?
    // Please increase the 'CLI_BUFFER_SIZE' in header file.
    // Or decrease max binding / history size.
    // You can get required buffer size by calling
    // uint16_t requiredSize = embeddedCliRequiredSize(config);
    // Then check it's value in debugger
    while (cli == NULL) {
        // HardFault_Handler();
    }

    // Un-comment to add a non-default reaction to unbound commands.
    //cli->onCommand = onCliCommand;

    // Add all the initial command bindings
    // manikin_cli_init();

    // Init the CLI with blank screen
    // manikin_clear_cli(cli, NULL, NULL);

    // CLI has now been initialized, set bool to true to enable interrupts.
    cliIsReady = true;
}

void cli_on_user_input(uint8_t *buf, uint32_t len) {
    for(size_t input_len = 0; input_len < len; input_len++) {
        embeddedCliReceiveChar(cli, buf[input_len++]);
    }
}

// Function to encapsulate the 'embeddedCliPrint()' call with print formatting arguments (act like printf(), but keeps cursor at correct location).
// The 'embeddedCliPrint()' function does already add a linebreak ('\r\n') to the end of the print statement, so no need to add it yourself.
void cli_printf(const char *format, ...) {
    // Create a buffer to store the formatted string
    char buffer[CLI_PRINT_BUFFER_SIZE];

    // Format the string using snprintf
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Check if string fitted in buffer else print error to stderr
    if (length < 0) {
        fprintf(stderr, "Error formatting the string\r\n");
        return;
    }

    // Call embeddedCliPrint with the formatted string
    embeddedCliPrint(getCliPointer(), buffer);
}

EmbeddedCli *getCliPointer() {
    return cli;
}
