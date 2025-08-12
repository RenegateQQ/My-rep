import os
import pytest
from unittest.mock import patch, MagicMock
from bot import *

@pytest.fixture
def example_message():
    """Fixture to simulate an example message."""
    class Message:
        def __init__(self, chat_id):
            self.chat = Chat(chat_id)
            self.text = ''

    class Chat:
        def __init__(self, chat_id):
            self.id = chat_id

    return Message(chat_id=123456789)  # Replace with a valid chat ID


def test_load_quiz_questions():
    """Test if loaded questions are present in the qviz.txt file."""
    # Load questions from the file
    loaded_questions = load_quiz_questions('qviz.txt')

    # Read questions from the qviz.txt file
    with open('qviz.txt', 'r', encoding='utf-8') as file:
        file_questions = [line.split('?')[0].strip() for line in file if line.strip()]

    # Check if each loaded question is present in the file
    for question, _ in loaded_questions:
        assert question in file_questions


def test_fetch_historical_events():
    expected_events = ("1903 – Românul de la Pind, the longest-running newspaper by and about Aromanians until World War II, is founded.\n"
                       "1908 – The first major commercial oil strike in the Middle East is made at Masjed Soleyman in southwest Persia. The rights to the resource were quickly acquired by the Anglo-Persian Oil Company.\n"
                       "1917 – Several powerful tornadoes rip through Illinois, including the city of Mattoon.\n"
                       "1918 – The Democratic Republic of Georgia is established.\n"
                       "1923 – The first 24 Hours of Le Mans is held and has since been run annually in June.\n"
                       "1927 – The last Ford Model T rolls off the assembly line after a production run of 15,007,003 vehicles.\n"
                       "1936 – In the House of Commons of Northern Ireland, Tommy Henderson begins speaking on the Appropriation bill. By the time he sits down in the early hours of the following morning, he had spoken for ten hours.\n"
                       "1937 – Walter Reuther and members of the United Auto Workers (UAW) clash with Ford Motor Company security guards at the River Rouge Complex complex in Dearborn, Michigan, during the Battle of the Overpass.\n"
                       "1938 – In the United States, the House Un-American Activities Committee begins its first session.\n"
                       "1940 – World War II: Operation Dynamo: In northern France, Allied forces begin a massive evacuation from Dunkirk, France.\n"
                       "1940 – World War II: The Siege of Calais ends with the surrender of the British and French garrison.\n"
                       "1942 – World War II: The Battle of Gazala takes place.\n"
                       "1948 – The U.S. Congress passes Public Law 80-557, which permanently establishes the Civil Air Patrol as an auxiliary of the United States Air Force.")

    events = fetch_historical_events('May 26')

    # Check if each expected event is present in the fetched events
    for expected_event in expected_events:
        assert expected_event in events

def test_fetch_random_article():
    """Test the fetch_random_article function."""
    article = fetch_random_article()
    assert isinstance(article, str)
    assert article != ''


def test_fetch_random_quote():
    """Test the fetch_random_quote function."""
    quote = fetch_random_quote()
    assert isinstance(quote, str)
    assert quote != ''

    # Check if the quote contains '*' at the beginning
    assert quote.lstrip().startswith('*')


def test_fetch_wikipedia_article():
    # Define the title of the Wikipedia article to fetch
    article_title = "Python (programming language)"

    # Define the expected content of the Wikipedia article
    expected_content = (
        "Python is a high-level, general-purpose programming language. Its design philosophy emphasizes code readability "
        "with the use of significant indentation.\n\n"
        "Python is dynamically typed and garbage-collected. It supports multiple programming paradigms, including structured "
        "(particularly procedural), object-oriented and functional programming. It is often described as a \"batteries included\" "
        "language due to its comprehensive standard library."
    )

    article_content = fetch_wikipedia_article(article_title)

    assert isinstance(article_content, str)
    assert article_content.strip() == expected_content.strip()

def test_fetch_images():
    """Test the fetch_images function."""
    # Define the title of the Wikipedia article to fetch images for
    article_title = "Python (programming language)"

    # Fetch images related to the Wikipedia article
    images = fetch_images(article_title)

    # Check if images were fetched
    assert images

    # Load the reference image
    reference_image_path = "/test_pictures/Python-logo-notext.svg.png"
    reference_image = Image.open(os.path.dirname(__file__) + reference_image_path)

    # Assert that each fetched image matches the reference image
    for image in images:
        # Convert the fetched image to PIL Image
        fetched_image = Image.open(image)

        # Compare the content of the fetched image with the reference image
        assert_image_equal(fetched_image, reference_image)

# Helper function to compare images
def assert_image_equal(image1, image2):
    assert image1.mode == image2.mode
    assert image1.size == image2.size
    assert image1.tobytes() == image2.tobytes()

def test_fetch_images_no_images():
    """Test the fetch_images function when no images are found."""
    # Define the title of the Wikipedia article with no images
    article_title = "Pavel"

    # Fetch images related to the Wikipedia article
    images = fetch_images(article_title)

    # Check if no images were fetched
    assert not images

def test_get_full_url_with_double_slash():
    """Test get_full_url function with a URL starting with double slashes."""
    url = '//example.com'
    expected_url = 'https://example.com'
    assert get_full_url(url) == expected_url

def test_get_full_url_with_single_slash():
    """Test get_full_url function with a URL starting with a single slash."""
    url = '/wiki/Page'
    expected_url = 'https://en.wikipedia.org/wiki/Page'
    assert get_full_url(url) == expected_url

def test_get_full_url_with_http_protocol():
    """Test get_full_url function with a URL starting with 'http://'."""
    url = 'http://example.com'
    expected_url = 'http://example.com'
    assert get_full_url(url) == expected_url

@patch('bot.bot')
def test_quiz_message_no_questions(mock_bot):
    """Test quiz_message function when no questions are available."""
    # Mock the message object
    message = MagicMock()

    # Mock the quiz_questions to be empty
    with patch('bot.quiz_questions', []):
        quiz_message(message)

    # Assert that the bot sends the appropriate message
    mock_bot.send_message.assert_called_once_with(message.chat.id, 'No quiz questions available.')

@patch('bot.bot')
@patch('bot.random.sample')
def test_quiz_message_with_questions(mock_sample, mock_bot):
    """Test quiz_message function when questions are available."""
    # Mock the message object
    message = MagicMock()

    # Mock the quiz_questions
    mock_questions = [('Question1', 'Answer1'), ('Question2', 'Answer2')]
    mock_sample.return_value = mock_questions

    # Call quiz_message function
    quiz_message(message)

    # Assert that the user_quiz_data is initialized correctly
    assert message.chat.id in user_quiz_data
    assert user_quiz_data[message.chat.id]['questions'] == mock_questions
    assert user_quiz_data[message.chat.id]['current_question'] == 0
    assert user_quiz_data[message.chat.id]['score'] == 0

@patch('bot.bot')
def test_ask_next_question(mock_bot):
    """Test ask_next_question function."""
    # Mock the message object
    message = MagicMock()

    # Mock the user_quiz_data
    user_quiz_data[message.chat.id] = {
        'questions': [('Question1', 'Answer1'), ('Question2', 'Answer2')],
        'current_question': 0,
        'score': 0
    }

    # Call ask_next_question function
    ask_next_question(message)

    # Assert that the bot sends the first question
    mock_bot.send_message.assert_called_once_with(message.chat.id, 'Question1')

@patch('bot.bot')
def test_end_quiz(mock_bot):
    """Test end_quiz function."""
    # Mock the message object
    message = MagicMock()

    # Mock the user_quiz_data
    user_quiz_data[message.chat.id] = {
        'questions': [('Question1', 'Answer1'), ('Question2', 'Answer2')],
        'current_question': 1,
        'score': 1
    }

    # Call end_quiz function
    end_quiz(message)

    # Assert that the user_quiz_data is removed
    assert message.chat.id not in user_quiz_data

    # Assert that the bot sends the correct score message
    mock_bot.send_message.assert_called_once_with(message.chat.id, 'Quiz finished! Your score: 1/2')

if __name__ == "__main__":
    pytest.main()
