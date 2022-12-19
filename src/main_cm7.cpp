#include "board.h"
#include "clock.h"
#include "dataset_loader.h"
#include "delay.h"
#include "file_system.h"
#include "logger.h"
#include "memory.h"
#include "multicore.h"
#include "profile.h"

#include "test_fast.h"
#include "test_lucas_kanade.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBER_OF_FEATURES_FOR_FAST (800)

/**
 * @brief Set when the secondary core has sent a message to this core.
 */
volatile bool has_received = false;

/**
 * @brief The message received from the secondary core.
 */
volatile multicore::Message received_message = {};

/**
 * @brief Buffer for the image data (placed in DTCM)
 */
static __attribute__((aligned(32))) uint8_t image_data[752 * 480];

/**
 * @brief Buffer for the lower levels of the image pyramid, placed in OCRAM3.
 */
SECTION_OCRAM3 static uint8_t lower_levels_image_pyramid_buffer[0x20000];

/**
 * @brief Start keypoints for Lucas-Kanade/keypoints extracted with fast.
 */
static image::KeyPoint keypoints[MAX_NUMBER_OF_FEATURES_FOR_FAST];

/**
 * @brief End position of the keypoints after a Lucas-Kanade track.
 */
static image::KeyPoint end_keypoints[MAX_NUMBER_OF_FEATURES_FOR_FAST];

/**
 * @brief Patch pyramid of the keypoints tracked, stored in OCRAM 1 and OCRAM 2.
 */
SECTION_OCRAM12 static image::PatchPyramid patch_pyramid;

/**
 * @brief Called when the secondary core sends a message to this core.
 */
void message_callback(const multicore::Message msg) {

    memcpy((void*)&received_message, (void*)&msg, sizeof(multicore::Message));
    has_received = true;
}

int main(void) {

    memory::configure_access_policy();

    clock::initialise();

    logger::initialise();
    logger::set_level(logger::Level::LOG_INFO);
    logger::set_prefix("(CORE0) ");

#ifdef DEBUG
    logger::rawf("\r\n");
    logger::infof("=== Starting up (Build type: DEBUG) ===\r\n");
#else
    logger::rawf("\r\n");
    logger::infof("=== Starting up (Build type: RELEASE) ===\r\n");
#endif

    logger::infof("Core clock: %d\r\n", SystemCoreClock);

    multicore::initialise();
    multicore::register_message_callback(message_callback);

    multicore::Message message;
    multicore::send_message(message);

    // Wait for the message from the secondary core that it has started up
    while (!has_received) { __asm volatile("nop"); }

    delay::ms(1000);

    logger::infof("Received message from second core: ");

    for (int i = 0; i < received_message.data[0]; i++) {
        logger::rawf("%c", (char)received_message.data[i + 1]);
    }

    logger::rawf("\r\n");

    if (!file_system::initialise()) {
        logger::errorf("Failed to initialise file system\r\n");
        exit(1);
    }

    // Testing FAST+LK against Vicon Room 2 03. Requires the dataset section
    // stored on the SD card in a directory called v23 where the images are
    // indexed sequentially
    logger::infof("V23 FAST + LK\r\n");
    test::lucas_kanade::test_with_dataset_without_references_with_resample(
        image_data,
        lower_levels_image_pyramid_buffer,
        &patch_pyramid,
        keypoints,
        end_keypoints,
        MAX_NUMBER_OF_PATCHES_IN_PYRAMID_LEVEL,
        "v23",
        1,
        1922);

    if (!file_system::deinitialise()) {
        logger::errorf("Failed to de-initialise file system\r\n");
    }

    return 0;
}
