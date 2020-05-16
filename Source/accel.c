#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#include "interception.h"

int read_file(double[]);
void temp_function_name(double[]);

int main(void)
{
  double variable[2];
  SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

  int success = read_file(variable);

  if (!success)
  {
    printf("FILE FAILED TO OPEN");
    return -1;
  }

  printf("Mouse Acceleration Starting.\nClose the program to stop the acceleration.\n");
  temp_function_name(variable);

  return 0;
}

int read_file(double vars[])
{
  FILE *stream; int i = 0; char temp[24];
  errno_t err = fopen_s(&stream, "settings.cfg", "r");

  if (err != 0) return 0;

  while (fscanf_s(stream, "%s", temp, _countof(temp)) == 1)
    fscanf_s(stream, "%lf", &vars[i++]);

  fclose(stream);
  return 1;
}

void temp_function_name(double vars[])
{
  double frame_time_ms = 0, dx, dy, accel_sense, rate, carry_x = 0, carry_y = 0;
  uint64_t frame_time, prev_frame_time, freq;
  InterceptionContext context = interception_create_context();
  InterceptionDevice device; InterceptionStroke stroke;

  interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);
  QueryPerformanceCounter(&prev_frame_time);
  QueryPerformanceFrequency(&freq);

  while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
  {
    if (interception_is_mouse(device))
    {
      InterceptionMouseStroke *mstroke = (InterceptionMouseStroke *)stroke;

      if (!(mstroke->flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE))
      {
        QueryPerformanceCounter(&frame_time);
        frame_time_ms = (double)(frame_time - prev_frame_time) * 1000.0 / freq;

        dx = (double)mstroke->x;
        dy = (double)mstroke->y;

        rate = sqrt(dx * dx + dy * dy) / frame_time_ms;
                
        rate *= vars[0];
        accel_sense = 1.0 + exp(log(rate));
        
        dx *= accel_sense;
        dy *= accel_sense;

        dx *= vars[1];
        dy *= vars[1];

        dx += carry_x;
        dy += carry_y;

        carry_x = dx - floor(dx);
        carry_y = dy - floor(dy);

        mstroke->x = (int)floor(dx);
        mstroke->y = (int)floor(dy);

        prev_frame_time = frame_time;
      }

      interception_send(context, device, &stroke, 1);
    }    
  }

  interception_destroy_context(context);
  return;
}
