#include "clock.h"
#include "delay.h"
#include "feature_extraction.h"
#include "led.h"
#include "linalg.h"
#include "logger.h"
#include "memory.h"
#include "multicore.h"
#include "profile.h"

volatile bool has_received                   = false;
volatile multicore::Message received_message = {};

void message_callback(const multicore::Message message) {

    memcpy((void*)&received_message,
           (void*)&message,
           sizeof(multicore::Message));
    has_received = true;
}

int main(void) {
    memory::configure_access_policy();

    clock::initialise();

    logger::initialise();
    logger::set_level(logger::Level::LOG_DEBUG);
    logger::set_prefix("(CORE1) ");

    multicore::initialise();
    multicore::register_message_callback(message_callback);

    led::init();

    logger::infof("Core clock: %d\r\n", SystemCoreClock);
    logger::debugf("Waiting for receive...\r\n");

    while (!has_received) {}

    logger::infof("Received ping from CORE0\r\n");

    multicore::Message message = {};

    const char* message_text = "Hello from second core";

    const size_t message_length = strlen(message_text);

    message.data[0] = (uint16_t)message_length;

    for (size_t i = 0; i < message_length; i++) {
        message.data[i + 1] = message_text[i];
    }

    multicore::send_message(message);

    while (true) {
        led::toggle();
        delay::ms(1000);
    }

    return 0;
}
