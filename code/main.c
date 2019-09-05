#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
/*--------------------------------------------------*/
unsigned char seconds = 0 ;
unsigned char minutes = 0 ;
unsigned char hours = 0 ;

unsigned char tick = 0 ;
unsigned char is_running = 1 ; /* to be used in Run/Pause Functionality */
/*--------------------------------------------------*/
/* External Interrupt INT0 To Clear Digital Clock */
void INT0_init(void)
{
	DDRD &= ~(1<<2) ;	 /* INT0 as Input */
	PORTD |= (1<<2) ;    /* Enable Internal Pull-Up */
	GICR |= (1<<6) ;     /* INT0 enable */
	MCUCR = (1<<ISC01) ; /* Trigger interrupt at falling edge */
}
ISR(INT0_vect)
{
	seconds = 0 ;
	minutes = 0 ;
	hours = 0 ;
}
/*--------------------------------------------------*/
/* External Interrupt INT1 is Used To RUN or PAUSE the Digital Clock */
void INT1_init(void)
{
	DDRD &= ~(1<<3) ; /* INT1 as Input */
	PORTD |= (1<<3) ; /* Enable Internal Pull Up */
	GICR |=(1<<7) ;   /* INT1 enable */
	MCUCR |= (1<<3) ; /* Trigger interrupt at falling edge */
}
ISR(INT1_vect)
{
	if(is_running == 1)
		{
			TCCR1B &= ~(1<<0) ;
			TCCR1B &= ~(1<<1) ;
			TCCR1B &= ~(1<<2) ;
			is_running = 0 ;
		}
		else
		{
			TCCR1B = (1<<0) | (1<<2) | (1<<WGM12) ;;
			is_running = 1 ;
		}
}
/*--------------------------------------------------*/
void Timer1_CTC_Init(unsigned short num_of_ticks )
{
	TCNT1 = 0 ; 		   					     /* Timer Initial Value */
	OCR1A = num_of_ticks ;						 /* Compare Value */
	TCCR1A = (1<<FOC1A) | (1<<FOC1B); 			 /* Non PWM , No Need For OC1A , OC1B */
	TIMSK |= (1<<OCIE1A) ;       				 /* output compare interrupt enable for channel A of Timer One */
	TCCR1B = (1<<CS10)|(1<<CS12)|(1<<WGM12) ;    /* Prescaler 1024 , ctc mode */
}
ISR(TIMER1_COMPA_vect)
{
	tick = 1 ;
}
/*--------------------------------------------------*/
int main(void)
{
	DDRC |= 0x0F ;
	PORTC &= 0xF0 ;

	DDRA |= 0x3F ;
	PORTA |= 0x3F ;

	SREG |=(1<<7) ;

	INT0_init() ;
	INT1_init() ;
	/* T for Timer = 1/(1/1024) = 1.024 ms
	   Compare value for 1s (1000ms) = 1000/1.024 = 976 */
	Timer1_CTC_Init(976) ;

	while(1)
	{
		if(tick == 1)
		{
			seconds++ ;
			if(seconds == 60 )
			{
				seconds = 0 ;
				minutes++ ;
			}
			if(minutes == 60 )
			{
				minutes = 0 ;
				hours++ ;
			}
			if(hours == 24)
			{
				hours = 0 ;
			}
			tick = 0 ;
		}
		else
		{
			/* display seconds */
			PORTA = (1<<5) ;
			PORTC = (PORTC&0xF0) | (seconds % 10) ;
			_delay_ms(2);
			PORTA = (1<<4) ;
			PORTC = (PORTC&0xF0) | (seconds / 10) ;
			_delay_ms(2) ;
			/* display minutes */
			PORTA = (1<<3) ;
			PORTC = (PORTC&0xF0) | (minutes % 10) ;
			_delay_ms(2);
			PORTA = (1<<2) ;
			PORTC = (PORTC&0xF0) | (minutes / 10) ;
			_delay_ms(2) ;
			/* display hours */
			PORTA = (1<<1) ;
			PORTC = (PORTC&0xF0) | (hours % 10) ;
			_delay_ms(2);
			PORTA = (1<<0) ;
			PORTC = (PORTC&0xF0) | (hours / 10) ;
			_delay_ms(2) ;
		}
	}
}
