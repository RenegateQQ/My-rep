"""
The main function of this bot is to search
and output wikipedia article and internet picture on demand.
The bot also has 4 buttons: "Quiz", "Historical retrospective", "Random article" and "Random quote".
When you click on "Quiz" the bot will start to output 5 questions from qviz.txt file,
when you click on "Random article" the bot will output one paragraph from a random wiki article,
when you click on "Historical retrospective" the bot will output a list of events that happened
on this day in the first half of the 20th century,
when you click on "Random quote" the bot will output a random quote
and also indicate where it came from.
"""
import random
from io import BytesIO
from datetime import datetime
import wikipediaapi
import requests
from PIL import Image
from PIL import UnidentifiedImageError
import telebot
from telebot import types
import wikipedia
import wikiquote
from bs4 import BeautifulSoup

TOKEN = "6858500129:AAELpICWrPwFuVo1XjDz7V2h0oINgupTDzQ"

wiki_wiki = wikipediaapi.Wikipedia('Wiki242_bot/1.0 (d96569755@gmail.com)', 'en')

bot = telebot.TeleBot(TOKEN)
def main_menu_markup():
    """
    Creates a custom keyboard markup for the main menu of the Telegram bot.

    This function generates a reply keyboard markup with four buttons:
    "Quiz", "Historical retrospective", "Random article", and "Random quote".
    :return:
        types.ReplyKeyboardMarkup: The markup object containing the custom keyboard layout.
    """
    markup = types.ReplyKeyboardMarkup(row_width=2, resize_keyboard=True)
    quiz_button = types.KeyboardButton('Quiz')
    historical_button = types.KeyboardButton('Historical retrospective')
    random_article_button = types.KeyboardButton('Random article')
    random_quote_button = types.KeyboardButton('Random quote')
    markup.add(quiz_button, historical_button,random_article_button,random_quote_button)
    return markup

@bot.message_handler(commands=['start'])
def start_message(message):
    """
    This function is triggered when the user starts a conversation with the bot.
    It sends a greeting message along with instructions on how to use the bot.
    :return:
        None
    """
    markup = main_menu_markup()
    bot.send_message(message.chat.id, 'Hello! Write me a name or event, '
        'and I will find a Wikipedia article for you.',reply_markup=markup)

@bot.message_handler(func=lambda message: message.text == 'Quiz')
def handle_quiz_command(message):
    """
    This function is triggered when the user selects the 'Quiz' option from the
    keyboard. It calls the `quiz_message` function to start quiz.
    :return:
        None
    """
    quiz_message(message)
def load_quiz_questions(filename='qviz.txt'):
    """
    This function loads the questions and answers from a text file.
    :return:
        A list of tuples where each tuple contains a question
        and its corresponding answer.
    """
    questions = []
    with open(filename, 'r', encoding='utf-8') as file:
        for line in file:
            parts = line.strip().split('?')
            if len(parts) == 2:
                question, answer = parts
                questions.append((question, answer))
    return questions

quiz_questions = load_quiz_questions()
user_quiz_data = {}
@bot.message_handler(commands=['quiz'])
def quiz_message(message):
    """
    This function is triggered when the user sends the '/quiz' command.
    It checks if there are quiz questions available. If not, it sends a message
    to the user indicating that no questions are available. Otherwise, it initializes
    the quiz data for the user, selecting a group of questions and setting the
    score and question index. Finally, it calls the function which asks the next question.
    :return:
        None
    """
    if not quiz_questions:
        bot.send_message(message.chat.id, 'No quiz questions available.')
        return

    user_quiz_data[message.chat.id] = {
        'questions': random.sample(quiz_questions, 5),
        'current_question': 0,
        'score': 0
    }
    ask_next_question(message)

def ask_next_question(message):
    """
    This function takes the current quiz data for the user and sends the next
    question in the list. If there are more questions remaining, it sends the next
    question to the user and checks the user's answer.
    If there are no more questions, it calls the function to end the quiz.
    :return:
        None
    """
    user_data = user_quiz_data.get(message.chat.id)
    if user_data and user_data['current_question'] < len(user_data['questions']):
        question, _ = user_data['questions'][user_data['current_question']]
        bot.send_message(message.chat.id, question)
        bot.register_next_step_handler(message, check_answer)
    else:
        end_quiz(message)

def check_answer(message):
    """
    This function compares the user's response to the correct answer for the
    current question. If the answer is correct, the user's score is incremented.
    After processing the answer, it updates the current question index and
    calls function which sends next question to either proceed to the next question or end the quiz.
    :return:
        None
    """
    user_data = user_quiz_data.get(message.chat.id)
    if not user_data:
        bot.send_message(message.chat.id, 'Quiz not found. '
                                          'Please start a new quiz by pressing the button.')
        return

    question_index = user_data['current_question']
    correct_answer = user_data['questions'][question_index][1]
    if message.text.strip().lower() == correct_answer.strip().lower():
        user_data['score'] += 1
        bot.send_message(message.chat.id, 'Correct!')
    else:
        bot.send_message(message.chat.id, f'Wrong! The correct answer is: {correct_answer}')

    user_data['current_question'] += 1
    ask_next_question(message)

def end_quiz(message):
    """
    This function is called when the quiz is completed. It takes and removes
    the user's quiz data. If quiz data exists
    for the user, it calculates the final score and sends a message with the
    user's score out of the total number of questions.
    :return:
        None
    """
    user_data = user_quiz_data.pop(message.chat.id, None)
    if user_data:
        score = user_data['score']
        bot.send_message(message.chat.id,
                         f'Quiz finished! Your score: {score}/{len(user_data["questions"])}')
@bot.message_handler(func=lambda message: message.text == 'Historical retrospective')
def historical_retrospective(message):
    """
    This function is triggered when the user clicks on the "Historical retrospective"
    button from the main menu. It takes the current date, constructs a Wikipedia
    page title using the month and day, and calls
    function to fetch historical events for that date. The retrieved events are
    then sent to the user as a message.
    :returns:
        None
    """
    today = datetime.now()
    month = today.strftime('%B')
    day = today.day
    page_title = f"{month} {day}"
    historical_events = fetch_historical_events(page_title)
    bot.send_message(message.chat.id, historical_events, reply_markup=main_menu_markup())

def fetch_historical_events(page_title: str) -> str:
    """
    This function takes a page title as input and retrieves historical events
    from the Wikipedia page. It specifically looks for events
    that occurred in period 1901-1950. If such events are found, it returns
    those events. If no events are found or if
    the page does not exist, it returns error messages.
    :returns:
        A string containing historical events that occurred in 1901-1950,
        or an error message if no events are found or if the page does not exist.
    """
    page = wiki_wiki.page(page_title)
    if page.exists():
        events_section = page.section_by_title('Events')
        if events_section:
            events_text = ""
            for subsection in events_section.sections:
                if subsection.title == '1901–present':
                    events_text += subsection.text.strip() + '\n'
            events_before_1950 = []
            for event in events_text.split('\n'):
                if event.strip():
                    year_str = event.split(' – ')[0].strip()
                    year = int(year_str) if year_str.isdigit() else None
                    if year and year < 1950:
                        events_before_1950.append(event)
            return '\n'.join(events_before_1950) if events_before_1950 else \
                'No historical events found before 1950.'
        return 'No historical events found for this date.'
    return 'Page not found.'


@bot.message_handler(func=lambda message: message.text == 'Random article')
def random_article(message):
    """
    This function is triggered when the user clicks the "Random article" button
    from the main menu. It fetches a random Wikipedia article, retrieves its summary,
    and sends the first paragraph of the summary to the user as a message.
    If no random article is found,an appropriate error message is sent to the user.
    :return:
        None
    """
    article = fetch_random_article()
    bot.send_message(message.chat.id, article, reply_markup=main_menu_markup())

def fetch_random_article() -> str:
    """
    This function generates a random Wikipedia article title and fetches the
    Wikipedia page. If the page exists, it retrieves the summary
    of the article and returns the first paragraph of the summary along with
    the title of the article highlighted by two asterisks on the sides.
    If no random article is found,an appropriate error message is returned.
    :returns:
        A string containing the first paragraph of a random Wikipedia article,
        or an error message if no article is found.
       """
    random_title = wikipedia.random(pages=1)
    page = wiki_wiki.page(random_title)
    if page.exists():
        summary = page.summary
        paragraphs = summary.split('\n')
        first_paragraph = [p for p in paragraphs if p.strip()][:1]
        return f"*{random_title}*\n\n" + '\n\n'.join(first_paragraph)
    return 'Random article not found.'


@bot.message_handler(func=lambda message: message.text == 'Random quote')
def random_quote(message):
    """
    This function is triggered when the user clicks the 'Random quote' button
    from the main menu. It fetches a random quote from Wikiquote, along with
    the name of the person or source associated with the quote, and sends it
    to the user as a message. If no random quote is found,
    an error message is sent to the user.
    :return:
        None
    """
    quote = fetch_random_quote()
    bot.send_message(message.chat.id, quote, reply_markup=main_menu_markup())

def fetch_random_quote() -> str:
    """
    This function generates a random title from Wikiquote, retrieves quotes
    associated with that title, and selects a random quote from the list.
    If a random quote is found, it returns the quote along with the source
    title. If no random quote is found,it returns an error message.
    :return:
        A string containing a random quote and its associated source title,
        or an error message if no quote is found.
    """
    titles = wikiquote.random_titles(max_titles=1, lang='en')
    if titles:
        random_title = random.choice(titles)
        quotes = wikiquote.quotes(random_title, lang='en')
        if quotes:
            return f"*Random Quote from {random_title}*\n\n" + random.choice(quotes)
    return 'No random quote found.'


@bot.message_handler(content_types=['text'])
def handle_message(message):
    """
    This function is triggered when the user sends a text message to the bot.
    It first fetches a Wikipedia article based on the user's query using the
    function. If the article is found, it sends the
    article content to the user as a message. If no article is found, it sends
    an error message to the user. Additionally, it fetches images
    related to the query using the fetch_images, and if images are
    found, it sends them to the user as photos. If no images are found, it
    sends a message of the absence of images.
    :return:
        None
    """
    query = message.text
    article = fetch_wikipedia_article(query)

    if article == 'Article not found.':
        bot.reply_to(message, 'Article not found.', reply_markup=main_menu_markup())
        return

    bot.reply_to(message, article)

    images = fetch_images(query)
    if images:
        for image in images:
            bot.send_photo(message.chat.id, photo=image)
    else:
        bot.reply_to(message, 'No images found.')

def fetch_wikipedia_article(title: str) -> str:
    """
    This function fetches a Wikipedia article using the provided title.
    If the article exists, it retrieves the summary of the article and
    returns the first two paragraphs. If the article does not exist,
    it returns an error message.
    :return:
        A string containing the first two paragraphs of the Wikipedia article's summary,
        or an error message if the article is not found.
    """
    page = wiki_wiki.page(title)
    if page.exists():
        summary = page.summary
        paragraphs = summary.split('\n')
        first_two_paragraphs = [p for p in paragraphs if p.strip()][:2]
        return '\n\n'.join(first_two_paragraphs)
    return 'Article not found.'


def fetch_images(title: str, timeout=30):
    """
    Fetches images from a Wikipedia page.
    :return:
         A list containing image content as BytesIO objects.
    Notes:
        The fetched images are converted to bytes using the Pillow library's
        Image module. This allows the images to be sent directly as photos
        in a Telegram message using the `send_photo`.
    """
    images = []
    try:
        page = wiki_wiki.page(title)
        if not page.exists():
            print(f"Page '{title}' does not exist.")
            return images

        html_content = requests.get(page.fullurl, timeout=timeout).text
        img_tag = find_main_image(html_content, timeout)

        if img_tag:
            image_bytes = download_image(img_tag, timeout)
            if image_bytes:
                images.append(image_bytes)
        else:
            print("No relevant image tag found on the page.")
    except requests.exceptions.RequestException as e:
        print(f"Error fetching page content: {e}")
    except UnidentifiedImageError as e:
        print(f"Error processing image: {e}")

    return images

def find_main_image(html_content, timeout):
    """
    Finds the main image from Wiki page, prioritizing an image from an infobox if available,
    and falling back to other images on the page.
    :return:
        If a main image is found, returns a dictionary containing HTML attributes of the image tag.
        If no relevant image is found, returns None.
    """
    soup = BeautifulSoup(html_content, 'html.parser')
    infobox = soup.find('table', {'class': 'infobox'})
    if infobox:
        img_tag = infobox.find('img')
        if img_tag:
            return img_tag

    img_tags = soup.find_all('img')
    for tag in img_tags:
        img_url = get_full_url(tag['src'])
        try:
            img_response = requests.get(img_url, timeout=timeout)
            img = Image.open(BytesIO(img_response.content))
            if is_relevant_image(img):
                return tag
        except UnidentifiedImageError:
            continue

    return None

def download_image(img_tag, timeout):
    img_url = get_full_url(img_tag['src'])
    headers = {'User-Agent': 'Mozilla/5.0'}
    img_response = requests.get(img_url, headers=headers, timeout=timeout)
    if img_response.status_code == 200:
        image = Image.open(BytesIO(img_response.content))
        image_bytes = BytesIO()
        image.save(image_bytes, format='PNG')
        image_bytes.seek(0)
        return image_bytes



def get_full_url(src):
    """
    Returns the full URL.
    :return:
        The full URL, ensuring it begins with 'https://' and includes the domain if necessary.
    Example:
        get_full_url('//example.com')  # returns 'https://example.com'
        get_full_url('/wiki/Page')     # returns 'https://en.wikipedia.org/wiki/Page'
        get_full_url('https://example.com')  # returns 'https://example.com' (unchanged)
    """
    if src.startswith('//'):
        return 'https:' + src
    if src.startswith('/'):
        return 'https://en.wikipedia.org' + src
    return src

def is_relevant_image(img):
    """
    Checks whether an image is relevant based on its dimensions.
    :return:
         True if the image's width and height are both greater or equal than 150 pixels,
         otherwise, False.
    """
    return img.width >= 120 and img.height >= 120

if __name__ == "__main__":
    bot.polling()
