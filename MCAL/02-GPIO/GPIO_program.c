/*******************************************************
 *******************************************************
 *****************  Author : Noha Elnemr ***************
 *****************  version : v1.0       ***************
 *****************  Date : 12/2/2023     ***************
 *******************************************************
 *******************************************************
 */

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "GPIO_interface.h"
#include "GPIO_cfg.h"
#include "GPIO_reg.h"

u8 GPIO_voidSetPinMode(u8 copy_u8Port, u8 copy_u8Pin , u8 copy_u8Mode)
{
	/* Initialize Error State Local Variable */
	   u8 Local_u8ErrorState = OK;
   switch(copy_u8Port)
   {
   case PORTA:
	     if(copy_u8Pin<=7)
	     {
	    	 GPIOA->CRL &= ~((0b1111)<<(copy_u8Pin *4));
	    	 GPIOA->CRL |= (copy_u8Mode<<(copy_u8Pin *4));
	     }
	     else if(copy_u8Pin<=15)
	     {
	    	 GPIOA->CRH &= ~((0b1111)<<((copy_u8Pin-8)*4));
	    	 GPIOA->CRH |= (copy_u8Mode<<((copy_u8Pin-8) *4));
	     }
	     else
	     {
	     	Local_u8ErrorState = NOK;
	     }
	   break;
   case PORTB:
	   if(copy_u8Pin<=7)
	   	     {
	   	    	 GPIOB->CRL &= ~((0b1111)<<(copy_u8Pin *4));
	   	    	 GPIOB->CRL |= (copy_u8Mode<<(copy_u8Pin *4));
	   	     }
	   	     else if(copy_u8Pin<=15)
	   	     {
	   	    	 GPIOB->CRH &= ~((0b1111)<<((copy_u8Pin-8)*4));
	   	    	 GPIOB->CRH |= (copy_u8Mode<<((copy_u8Pin-8) *4));
	   	     }
	   	  else
	   	     {
	   	  	   	Local_u8ErrorState = NOK;
	   	  	 }
   	   break;
   case PORTC:
	   if(copy_u8Pin<=7)
	   	     {
	   	    	 GPIOC->CRL &= ~((0b1111)<<(copy_u8Pin *4));
	   	    	 GPIOC->CRL |= (copy_u8Mode<<(copy_u8Pin *4));
	   	     }
	   	     else if(copy_u8Pin<=15)
	   	     {
	   	    	 GPIOC->CRH &= ~((0b1111)<<((copy_u8Pin-8)*4));
	   	    	 GPIOC->CRH |= (copy_u8Mode<<((copy_u8Pin-8) *4));
	   	     }
	   	  else
	   	  	  {
	   	  	    Local_u8ErrorState = NOK;
	   	  	  }
   	   break;

      default :  Local_u8ErrorState = NOK; break;		    /* Wrong Choice In Copy_u8Port */
   	}

   	return Local_u8ErrorState ;
}

u8 GPIO_voidSetPinValue(u8 copy_u8Port, u8 copy_u8Pin , u8 copy_u8Value)
{
	/* Initialize Error State Local Variable */
		u8 Local_u8ErrorState = OK;
		switch(copy_u8Port)
		{
		case PORTA :
			if(copy_u8Value == GPIO_HIGH)
			{
				GPIOA->BSRR = (1<<copy_u8Pin);
			}
			else if(copy_u8Value == GPIO_LOW)
			{
				GPIOA->BRR = (1 << copy_u8Pin);
			}
			else
			{
				Local_u8ErrorState = NOK;
			}
			break;

		case PORTB :
			if(copy_u8Value == GPIO_HIGH)
			{
				GPIOB->BSRR = (1<<copy_u8Pin);
			}
		   else if(copy_u8Value == GPIO_LOW)
			{
			   GPIOB->BRR = (1 << copy_u8Pin);
			}
			else
			{
				Local_u8ErrorState = NOK;
			}
			break;

		case PORTC :
			if(copy_u8Value == GPIO_HIGH)
			{
				GPIOC->BSRR = (1<<copy_u8Pin);
			}
			else if(copy_u8Value == GPIO_LOW)
			{
				GPIOC->BRR = (1 << copy_u8Pin);
			}
			else
			{
				Local_u8ErrorState = NOK;
			}
			break;

		default :  Local_u8ErrorState = NOK; break;
		}
		return Local_u8ErrorState;
}
u8 GPIO_voidGetPinValue(u8 copy_u8Port, u8 copy_u8Pin)
{
   u8 Local_u8pinvalue=0;
   switch(copy_u8Port)
   {
   case PORTA:
	   Local_u8pinvalue=GET_BIT(GPIOA->IDR,copy_u8Pin);
	   break;
   case PORTB:
	   Local_u8pinvalue=GET_BIT(GPIOB->IDR,copy_u8Pin);
   	   break;
   case PORTC:
	   Local_u8pinvalue=GET_BIT(GPIOC->IDR,copy_u8Pin);
   	   break;
   }
   return  Local_u8pinvalue;
}

u8 GPIO_u8SetPortValue(u8 Copy_u8Port , u16 Copy_u16Value)
{
	/* Initialize Error State Local Variable */
		u8 Local_u8ErrorState = OK;

		switch(Copy_u8Port)
		{
		case PORTA	:	GPIOA->ODR = Copy_u16Value;	break;
		case PORTB	:	GPIOB->ODR = Copy_u16Value;	break;
		case PORTC	:	GPIOC->ODR = Copy_u16Value;	break;
		default   	: 	Local_u8ErrorState = NOK;	break;
		}
		return Local_u8ErrorState;
}





