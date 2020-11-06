#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/cm3/nvic.h>

#define LED_PIN GPIO15
#define LED_PORT GPIOC

#define LED0_PORT GPIOA
#define LED0_PIN GPIO0
#define LED1_PORT GPIOA
#define LED1_PIN GPIO1
#define LED2_PORT GPIOB
#define LED2_PIN GPIO1
#define LED3_PORT GPIOB
#define LED3_PIN GPIO6

#define BTN_PORT GPIOA
#define BTN_PIN GPIO4

// TIM21 = 65536/4096 ~= 16 Hz Count increment
#define TIM21_INT_FREQ 4

enum LightState
{
    ON = 0,
    BLINK = 1,
    CHASE = 2,
    ALTERNATE = 3,
    NUM_STATES 
};

volatile uint32_t light_state;
volatile uint32_t tim21_count;

void tim21_isr(void)
{
    timer_clear_flag(TIM21, TIM_SR_CC1IF);

    switch (light_state)
    {
    case ON:
        timer_set_oc_value(TIM2, TIM_OC1, 1);
        timer_set_oc_value(TIM2, TIM_OC2, 1);
        timer_set_oc_value(TIM2, TIM_OC3, 1);
        timer_set_oc_value(TIM2, TIM_OC4, 1);
        break;

    case BLINK:
        if (tim21_count % 2)
        {
            timer_set_oc_value(TIM2, TIM_OC1, 1);
            timer_set_oc_value(TIM2, TIM_OC2, 1);
            timer_set_oc_value(TIM2, TIM_OC3, 1);
            timer_set_oc_value(TIM2, TIM_OC4, 1);
        }
        else
        {
            timer_set_oc_value(TIM2, TIM_OC1, 0);
            timer_set_oc_value(TIM2, TIM_OC2, 0);
            timer_set_oc_value(TIM2, TIM_OC3, 0);
            timer_set_oc_value(TIM2, TIM_OC4, 0);
        }
        break;

    case CHASE:
        switch (tim21_count % 4)
        {
        case 0:
            timer_set_oc_value(TIM2, TIM_OC1, 1);
            timer_set_oc_value(TIM2, TIM_OC2, 0);
            timer_set_oc_value(TIM2, TIM_OC3, 0);
            timer_set_oc_value(TIM2, TIM_OC4, 0);
            break;
        case 1:
            timer_set_oc_value(TIM2, TIM_OC1, 0);
            timer_set_oc_value(TIM2, TIM_OC2, 1);
            timer_set_oc_value(TIM2, TIM_OC3, 0);
            timer_set_oc_value(TIM2, TIM_OC4, 0);
            break;
        case 2:
            timer_set_oc_value(TIM2, TIM_OC1, 0);
            timer_set_oc_value(TIM2, TIM_OC2, 0);
            timer_set_oc_value(TIM2, TIM_OC3, 1);
            timer_set_oc_value(TIM2, TIM_OC4, 0);
            break;
        case 3:
            timer_set_oc_value(TIM2, TIM_OC1, 0);
            timer_set_oc_value(TIM2, TIM_OC2, 0);
            timer_set_oc_value(TIM2, TIM_OC3, 0);
            timer_set_oc_value(TIM2, TIM_OC4, 1);
            break;
        default:
            break;
        }
        break;

    case ALTERNATE:
        if (tim21_count % 2)
        {
            timer_set_oc_value(TIM2, TIM_OC1, 0);
            timer_set_oc_value(TIM2, TIM_OC2, 1);
            timer_set_oc_value(TIM2, TIM_OC3, 0);
            timer_set_oc_value(TIM2, TIM_OC4, 1);
        }
        else
        {
            timer_set_oc_value(TIM2, TIM_OC1, 1);
            timer_set_oc_value(TIM2, TIM_OC2, 0);
            timer_set_oc_value(TIM2, TIM_OC3, 1);
            timer_set_oc_value(TIM2, TIM_OC4, 0);
        }
        break;

    default:
        break;
    }

    tim21_count = (tim21_count + 1) % 4;
}

void exti4_15_isr(void)
{
    exti_reset_request(EXTI4);
    gpio_toggle(LED_PORT, LED_PIN);
    light_state = (light_state + 1) % NUM_STATES;
}

int main(void)
{
    // Set clock to internal clock at lowest speed
    rcc_set_msi_range(RCC_ICSCR_MSIRANGE_65KHZ);

    rcc_set_sysclk_source(RCC_MSI);

    rcc_set_hpre(RCC_CFGR_HPRE_NODIV);

    rcc_set_ppre1(RCC_CFGR_PPRE1_NODIV);
    rcc_set_ppre2(RCC_CFGR_PPRE2_NODIV);

    // Initialize peripherals access to clock
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);

    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_TIM21);

    // Setup GPIO
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);

    gpio_mode_setup(LED0_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LED0_PIN);
    gpio_mode_setup(LED1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LED1_PIN);
    gpio_mode_setup(LED2_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LED2_PIN);
    gpio_mode_setup(LED3_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LED3_PIN);

    gpio_set_af(LED0_PORT, GPIO_AF2, LED0_PIN);
    gpio_set_af(LED1_PORT, GPIO_AF2, LED1_PIN);
    gpio_set_af(LED2_PORT, GPIO_AF5, LED2_PIN);
    gpio_set_af(LED3_PORT, GPIO_AF5, LED3_PIN);

    gpio_set_output_options(LED0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, LED0_PIN);
    gpio_set_output_options(LED1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, LED1_PIN);
    gpio_set_output_options(LED2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, LED2_PIN);
    gpio_set_output_options(LED3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, LED3_PIN);

    // TIM2 Setup for PWM on all channels
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    timer_set_prescaler(TIM2, 0);
    timer_continuous_mode(TIM2);
    timer_set_period(TIM2, 1024);

    timer_disable_oc_output(TIM2, TIM_OC1);
    timer_disable_oc_output(TIM2, TIM_OC2);
    timer_disable_oc_output(TIM2, TIM_OC3);
    timer_disable_oc_output(TIM2, TIM_OC4);
    timer_disable_oc_clear(TIM2, TIM_OC1);
    timer_disable_oc_clear(TIM2, TIM_OC2);
    timer_disable_oc_clear(TIM2, TIM_OC3);
    timer_disable_oc_clear(TIM2, TIM_OC4);
    timer_enable_oc_preload(TIM2, TIM_OC1);
    timer_enable_oc_preload(TIM2, TIM_OC2);
    timer_enable_oc_preload(TIM2, TIM_OC3);
    timer_enable_oc_preload(TIM2, TIM_OC4);
    timer_set_oc_slow_mode(TIM2, TIM_OC1);
    timer_set_oc_slow_mode(TIM2, TIM_OC2);
    timer_set_oc_slow_mode(TIM2, TIM_OC3);
    timer_set_oc_slow_mode(TIM2, TIM_OC4);
    timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
    timer_set_oc_mode(TIM2, TIM_OC2, TIM_OCM_PWM1);
    timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_PWM1);
    timer_set_oc_mode(TIM2, TIM_OC4, TIM_OCM_PWM1);
    timer_set_oc_value(TIM2, TIM_OC1, 1);
    timer_set_oc_value(TIM2, TIM_OC2, 1);
    timer_set_oc_value(TIM2, TIM_OC3, 1);
    timer_set_oc_value(TIM2, TIM_OC4, 1);
    timer_enable_oc_output(TIM2, TIM_OC1);
    timer_enable_oc_output(TIM2, TIM_OC2);
    timer_enable_oc_output(TIM2, TIM_OC3);
    timer_enable_oc_output(TIM2, TIM_OC4);

    timer_enable_counter(TIM2);

    // TIM21 Setup for IRQ to update TIM2 CCR values
    nvic_enable_irq(NVIC_TIM21_IRQ);
    rcc_periph_reset_pulse(RST_TIM21);
    timer_set_mode(TIM21, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_disable_preload(TIM21);
    timer_continuous_mode(TIM21);
    timer_set_prescaler(TIM21, 4095);
    timer_set_period(TIM21, 16/TIM21_INT_FREQ);
    timer_enable_counter(TIM21);
    timer_enable_irq(TIM21, TIM_DIER_CC1IE);

    // EXTI setup for push button to toggle modes
    gpio_mode_setup(BTN_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, BTN_PIN);
    exti_select_source(EXTI4, BTN_PORT);
    exti_set_trigger(EXTI4, EXTI_TRIGGER_RISING);
    exti_enable_request(EXTI4);
    nvic_enable_irq(NVIC_EXTI4_15_IRQ);

    while (1)
    {
        // Attempt to save power, although most power saving 
        // is coming from the low clock speed
        pwr_set_stop_mode();
    }

    return 0;
}
