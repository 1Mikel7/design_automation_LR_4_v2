#include <stdint.h>
#include "system.h"
#include "io.h"
#include "sys/alt_stdio.h"

#define STATE_OFFSET 0x00
#define CTRL_OFFSET  0x04
#define NEXT_COMMAND 1u
#define PHASE_COUNT  7u

static void show_state(uint32_t state)
{
    // Младшие три бита описывают светофор A, биты 16–18 — светофор B.
    uint32_t lamps_a = state & 0x7u;
    uint32_t lamps_b = (state >> 16) & 0x7u;
    uint32_t red = 0;
    uint32_t green = 0;

    // Для демонстрации используем LEDR/LEDG 2 для A и LEDR/LEDG 3 для B.
    // Жёлтый показываем одновременным включением красного и зелёного.
    if (lamps_a & 0x3u) red |= 1u << 2;
    if (lamps_a & 0x6u) green |= 1u << 2;
    if (lamps_b & 0x3u) red |= 1u << 3;
    if (lamps_b & 0x6u) green |= 1u << 3;

    IOWR_32DIRECT(PIO_RED_BASE, 0, red);
    IOWR_32DIRECT(PIO_GREEN_BASE, 0, green);
}

int main(void)
{
    static const uint32_t expected[PHASE_COUNT] = {
        0x00010001, 0x00010003, 0x00010004, 0x00010002,
        0x00030001, 0x00040001, 0x00020001
    };
    unsigned int step;

    alt_putstr("APB traffic light test\n");

    // Читаем начальную фазу и состояние после каждой команды NEXT.
    for (step = 0; step <= PHASE_COUNT; ++step) {
        uint32_t state = IORD_32DIRECT(APB_TRAFFIC_LIGHT_0_BASE, STATE_OFFSET);
        uint32_t expected_state = expected[step % PHASE_COUNT];

        alt_printf("step %x: STATE=0x%x\n", step, (unsigned int)state);
        if (state != expected_state) {
            alt_printf("FAIL: expected 0x%x\n", (unsigned int)expected_state);
            return 1;
        }

        show_state(state);

        if (step < PHASE_COUNT) {
            IOWR_32DIRECT(APB_TRAFFIC_LIGHT_0_BASE, CTRL_OFFSET, NEXT_COMMAND);
        }
    }

    alt_putstr("PASS: all seven phases and return to start\n");
    return 0;
}
