/**
 * @file EPDLite.h
 * @brief ePaper Display Interface
 * @ingroup  EPDLite
 * @addtogroup  EPDLite
 * \{
 */

#ifndef EPDLITE_EPDLITE_H_INCLUDE
#define EPDLITE_EPDLITE_H_INCLUDE

#include <stddef.h>
#include <stdint.h>

#include <SPI.h>

#include "EPDLite/commands.h"


/**
 * @brief Public interface to the @see CommandBuffer
 * @details Provides a public interface to the CommandBuffer to allow polymorphic use of CommandBuffer with template values
 */
class CommandBufferInterface
{
public:
  /**
   * @brief The current number of commands stored in this buffer
   * @return size
   */
  virtual size_t size() const;
  /**
   * @brief The maximum number of commands that can be stored in this buffer (i.e., TCommandSize it was created with).
   * @return capacity
   */
  virtual size_t capacity() const;

  /**
   * @brief Removed a command from the end of the buffer
   * @details No operation if there is no command to remove
   */
  virtual void pop() = 0;

  virtual uint8_t process(const size_t at, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd) = 0;

  /**
   * @brief The maximum amount of memory used for a single command.
   * @details When constructing an instance of CommandBuffer, this can be used for TCommandSize.
   */
  static constexpr size_t max_size() {
    return max_sizeof<
      PixelCommand,
      LineCommand,
      RectCommand,
      CircleCommand,
      TextCommand,
      BufferCommand
    >();
  }

private:
  template <typename T>
  static constexpr T static_max(T a, T b)
  {
    return a < b ? b : a;
  }

  template <typename T, typename... Ts>
  static constexpr T static_max(T a, Ts... bs)
  {
    return static_max(a, static_max(bs...));
  }

  template <typename... Ts>
  static constexpr size_t max_sizeof()
  {
    return static_max(sizeof(Ts)...);
  }
};

/**
 * @brief A buffer to store commands to render
 * @details The buffer is statically allocated on the stack, commands can be pushed into the buffer to be rendered on the display
 *
 * @tparam TCommandCount The maximum number of commands allowed in this buffer.
 * @tparam TCommandSize The maximum size of a command allowed in this buffer. Suggested to use MAX_COMMAND_SIZE, but smaller values can be used for memory reduction.
 */
template <size_t TCommandCount, size_t TCommandSize = CommandBufferInterface::max_size()>
class CommandBuffer : public CommandBufferInterface
{
public:
  CommandBuffer() : CommandBufferInterface(), count(0)
  {
  }

  /**
   * @brief The current number of commands stored in this buffer
   * @return size
   */
  virtual size_t size() const { return count; }
  /**
   * @brief The maximum number of commands that can be stored in this buffer (i.e., TCommandSize it was created with).
   * @return capacity
   */
  virtual size_t capacity() const { return TCommandCount; }

  /**
   * @brief Add a command to the buffer
   * @details Pushes a command onto the end of the buffer
   *
   * @tparam TCommand The type of command to push
   * @param command
   */
  template <typename TCommand>
  void push(TCommand command)
  {
    static_assert(sizeof(TCommand) <= TCommandSize, "Pushed command is bigger. Increase TCommandCount.");

    if (count >= TCommandCount)
      return; // need to handle this somehow?

    memcpy(&commands[TCommandSize * count], &command, sizeof(TCommand));
    call_table[count++] = &TCommand::process;
  }

  /**
   * @brief Removed a command from the end of the buffer
   * @details No operation if there is no command to remove
   */
  virtual void pop()
  {
    if (count > 0)
      --count;
  }

  /**
   * @brief Dispatches the process to the correct command instance.
   *
   * @param at The command in the buffer to run
   * @param input The current 8 pixel screen data
   * @param x The x position of the pixel data
   * @param y The y position of the pixel data
   * @return The modified 8 pixels
   */
  virtual uint8_t process(const size_t at, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd) override
  {
    return (call_table[at])((void*)&commands[at * TCommandSize], input, x, y, epd);
  }


private:
  uint8_t (*call_table[TCommandCount])(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd);

  uint8_t commands[TCommandCount * TCommandSize];
  size_t count;
};


using pin_t =  int8_t;

/**
 * @brief Controls an ePaper Display
 *
 */
class EPDLite
{
public:
  /**
   * @brief Initializes the display.
   *
   * @param w Width of the display in pixels
   * @param h Height of the dusplay in pixels
   * @param cs Chip Select pin
   * @param dc Data/Command pin
   * @param busy Busy pin
   * @param reset Reset pin
   */
  EPDLite(const int16_t w, const int16_t h, const pin_t cs, const pin_t dc, const pin_t busy, const pin_t reset);


  void setOrientation(const uint8_t o);
  uint8_t getOrientation() const { return orientation; }

  /**
   * @brief Initializes the display
   */
  void init();

  /**
   * @brief Resets the display
   * @details Performs a hard reset (using the reset pin) and then a soft reset.
   */
  void reset();

  /**
   * @brief Loads a waveform
   * @details ePaper displays sometimes require a waveform so that they can be controlled properly to show the right screen contents. Waveform data for your display will be specific to the manufacturing batch and can be obtained from the manufacturer
   *
   * @param waveform the waveform to load
   * @param len the length of the waveform
   */
  void loadLUT(uint8_t* waveform, size_t len);

  /**
   * @brief Render to the display from the command buffer
   *
   * @param buffer Renders the commands listed into the buffer onto a blank screen.
   * @param doBlock Blocks until the render is complete, if false call `wait` before sending any commands to the display again.
   */
  void render(CommandBufferInterface& buffer, const bool doBlock = true);

  /**
   * @brief Render to the display the raw data from the buffer
   * @details Rendering using a full memory buffer, similar to other libraries. The buffer must be big enough to hold the entire screen contents.
   *
   * @param buffer The buffer must be at least `height * width / 8` bytes. UB if it's not.
   * @param doBlock Blocks until the render is complete, if false call `wait` before sending any commands to the display again.
   */
  void render(const uint8_t* const buffer, const bool doBlock = true);

  /**
   * @brief Render to the display from a buffer storee in PROGMEM
   * @details Rendering using a full memory buffer from flash. The buffer must be big enough to store an entire image.
   *
   * @param buffer The buffer in PROGMEM
   * @param doBlock Blocks until the render is complete, if false call `wait` before sending any commands to the display again.
   */
  void render_P(const uint8_t* const buffer, const bool doBlock = true);

  /**
   * @brief Waits until the display's busy line is low
   */
  inline void wait() { preblock(); }

  /**
   * @brief Returns true if the display's busy line is low
   */
  inline bool ready() { return !digitalRead(pin_busy); }

  /**
   * @brief Blanks the display
   */
  void clear();

  /**
   * @brief The width of the display in pixels
   */
  const int16_t width;
  /**
   * @brief The height of the display in pixels
   */
  const int16_t height;

private:
  /**
   * @brief blocks execution until the busy pin indicates the display is ready
   */
  void block();

  /**
   * @brief blocks execution until the busy pin indicates the display is ready
   * checks pin state before blocking
   */
  void preblock();

  /**
   * @brief sets the display's ram address pointer
   *
   * @param x The x coordinate
   * @param y The y coordinate
   */
  void place(const int16_t x, const int16_t y);

  /**
   * @brief Sends a command to the display
   * @details Switches the data/command pin into command mode and writes the command provided
   *
   * @param c
   */
  void command(const uint8_t c);

  /**
   * @brief Sends a byte of data to the display
   * @details Switches the data/command pin into data mode and writes the data provided
   *
   * @param d
   */
  void data(const uint8_t d);

  /**
   * @brief Sends a buffer of data to the display
   * @details Switches the data/command pin into data mode and writes the data provided
   *
   * @param d The data to send
   * @param len The number of bytes to send
   */
  void data(const uint8_t* const d, const size_t len);

  const pin_t pin_cs;
  const pin_t pin_dc;
  const pin_t pin_busy;
  const pin_t pin_reset;

  const SPISettings settings;

  uint8_t orientation;

  static const uint8_t DATA_ENTRY_ORDER = 0x11;

  static const uint8_t Y_INC = 0b10;
  static const uint8_t X_INC = 0b01;
  static const uint8_t UPDATE_X = 0b000;
  static const uint8_t UPDATE_Y = 0b100;

  static const uint8_t SOFT_RESET = 0x12;

  static const uint8_t WRITE_RAM = 0x24;

  static const uint8_t SET_X_SIZE = 0x44;
  static const uint8_t SET_Y_SIZE = 0x45;
  static const uint8_t SET_X_ADDRESS = 0x4E;
  static const uint8_t SET_Y_ADDRESS = 0x4F;
  static const uint8_t DISPLAY_UPDATE_CONTROL = 0x21;
  static const uint8_t DISPLAY_UPDATE_SEQUENCE = 0x20;
};

#endif

/* \} */
