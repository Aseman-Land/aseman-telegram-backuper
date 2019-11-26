# Aseman Telegram Backuper
Simple GUI Qt application that take backup from your messages and medias using libqtelegram

## Diary keys

| Key                    | Type | Description                                              |
| ---------------------- | ---- | -------------------------------------------------------- |
| avgContiniusMessages   | real | Average of messages sent continiusly                     |
| avgDelay               | real | Average Delay between two messages per seconds           |
| avgDelayPerSession     | real | Average Delay between two sessions per seconds           |
| avgEmojiPerMessage     | real | Average Emoji count per messages                         |
| avgLongDelay           | real | Average Long Delay (Delays longer than 1h) per seconds   |
| avgLongDelayPerSession | real | Average Long Session Delay (Longer than 1h) per seconds  |
| avgPerMessageCharCount | real | Average characters per message                           |
| avgRoundDuration       | real | Average duration of rounded videos per seconds           |
| avgSmallDelay          | real | Average Small Delay (Delays smaller than 1h) per seconds |
| avgSmallDelayPerSession | real | Average Small Session Delay (Smaller than 1h) per seconds |
| avgVoiceDuration       | real | Average duration of voice messages per seconds           |
| charCount              | int  | Count of characters                                      |
| count                  | int  | Count of all messages                                    |
| delay                  | int  | Sum of all delays per seconds                            |
| delayPerSession        | int  | Sum of all delays between two sessions per seconds       |
| emojiMessagesCount     | int  | Count of Messages, Contains emojis                       |
| emojisCount            | int  | Count of all emojis                                      |
| imageCount             | int  | Count of images                                          |
| longDelay              | int  | Sum of Long Delays (Delays longer than 1h) per seconds   |
| longDelayPerSession    | int  | Sum of Long Session Delays (Longer than 1h) per seconds  |
| maxDelay               | int  | Maximum delay between two messages per seconds           |
| maxDelayPerSession     | int  | Maximum delay between two sessions per seconds           |
| messageSessionsCount   | int  | Count of Sessions (A session is messages sent coninusly) |
| minDelay               | int  | Minimum delay between two messages per seconds           |
| minDelayPerSession     | int  | Minimum delay between two sessions per seconds           |
| percentContaintsEmoji  | real | Percent of messages contains emoji                       |
| percentRound           | real | Percent of messages is rounded video                     |
| percentVoice           | real | Percent of messages is Voice Message                     |
| roundCount             | int  | Count of all rounded videos                              |
| roundDuration          | int  | Duration of all rounded videos per seconds               |
| smallDelay             | int  | Sum of Small Delays (Delays smaller than 1h) per seconds |
| smallDelayPerSession   | int  | Sum of Small Session Delays (Smaller than 1h) per seconds |
| stickerPerChar         | map  | Map of sticker emojis                                    |
| stickers               | int  | Count of stickers                                        |
| usedEmojis             | map  | Map of emojis                                            |
| voiceCount             | int  | Count of all voice messages                              |
| voiceDuration          | int  | Duration of all voice messages per seconds               |

