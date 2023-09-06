from typing import final
import re
from telegram import Update
from firebase_admin import credentials, initialize_app, firestore
from telegram.ext import Application, CommandHandler, MessageHandler, CallbackContext, filters, ContextTypes

TOKEN: final = '6144245606:AAHWOvz_wcVmE_qBkgGxbDNfTx_57S2LbPo'
BOT_USERNAME: final = '@BAS_FB_bot'

cred = credentials.Certificate('biometric-attendance-sys-1deca-firebase-adminsdk-wi89e-f51e9a95d5.json')
initialize_app(cred)

def is_valid_message(message):
    pattern = r'finger print id is: \d+ and user id is: \d+'
    return re.match(pattern, message) is not None

#commands
async def start_command(update: Update, context: ContextTypes.DEFAULT_TYPE):
    await update.message.reply_text('hello!, i am the BOSS!')

async def check_firestore(update: Update, context: CallbackContext):
    # Read data from Firebase
    db = firestore.client()
    collection_name = 'PendingUsers'

    # Check if the collection exists and is not empty
    collection_ref = db.collection(collection_name)
    documents = collection_ref.stream()
    if len(list(documents)) == 0:
        await update.message.reply_text("The collection is empty or does not exist.")
        return
    for doc in documents:
        # Get data from the current document
        document_data = doc.to_dict()

        if document_data:
            fingerID = document_data.get('data', {}).get('FingerPrintID')
            userID = document_data.get('data', {}).get('id')
            time = document_data.get('data', {}).get('time')
        if fingerID is not None and userID is not None :
            print(f"finger print id is:{fingerID} and user id is {userID} registered at: {time}")
            await update.message.reply_text(f"finger print id is: {fingerID} and user id is: {userID} registered at: {time}")
        else:
            print("Field not found or is empty")

async def showApprovedUsers(update: Update, context: CallbackContext):
    db = firestore.client()
    collection_name = 'ApprovedUsers'
    collection_ref = db.collection(collection_name)
    documents = collection_ref.stream()
    if len(list(documents)) == 0:
        update.message.reply_text("The collection is empty or does not exist.")
        return
    for doc in documents:
        # Get data from the current document
        document_data = doc.to_dict()
        if document_data:
            fingerID = document_data.get('data', {}).get('FingerPrintID')
            userID = document_data.get('data', {}).get('id')
            time = document_data.get('data', {}).get('time')
        if fingerID is not None and userID is not None:
            print(f"finger print id:{fingerID} \nid:{userID} \ntime: {time}")
            await update.message.reply_text(f"finger print id:{fingerID} \nid:{userID} \ntime: {time}")
        else:
            print("Field not found or is empty")

#Responses
def handle_response(text: str) -> str:
    processed: str = text.lower()
    if 'hello' in processed:
        return 'hey there'
    elif 'how are you' in processed:
        return 'i am good'
    else: return 'BAKAAAAA'

def handle_approval(text):
    db = firestore.client()
    print(text)
    words = text.split()
    finger_print_id = None
    user_id = None
    time = None
    # Iterate through the words to find the values
    for i in range(len(words) - 1):
        if words[i] == "is:":
            if words[i - 2] == "print":
                finger_print_id = words[i + 1]
            elif words[i - 2] == "user":
                user_id = words[i + 1]
        elif words[i] == "at:":
            time = words[i+1]

    if finger_print_id is not None:
        finger_print_id = int(finger_print_id)

    user_data = {
        'data': {
            'FingerPrintID': finger_print_id,
            'id': user_id,
            'time': time
        }
    }
    #add user to approval collection
    approved_users_collection = db.collection('ApprovedUsers')
    document_id = f'user{finger_print_id}'
    approved_users_collection.document(document_id).set(user_data)

    #delete user from pending cillection
    pendingList = db.collection('PendingUsers')
    document = f'user{finger_print_id}'
    document_ref = pendingList.document(document)
    document_ref.delete()

def handle_decline(text):
    words = text.split()
    finger_print_id = None

    for i in range(len(words) - 1):
        if words[i] == "is:":
            if words[i - 2] == "print":
                finger_print_id = words[i + 1]

    if finger_print_id is not None:
        finger_print_id = int(finger_print_id)

    db = firestore.client()
    pendingList = db.collection('PendingUsers')
    document = f'user{finger_print_id}'
    document_ref = pendingList.document(document)
    document_ref.delete()
    print(f"user{finger_print_id} has been deleted from PendingUsers collection")

async def handle_message(update: Update, context: ContextTypes.DEFAULT_TYPE):
    message_type: str = update.message.chat.type
    text: str = update.message.text

    print(f'User ({update.message.id}) in {message_type}: "{text}"')

    if message_type =='group':
        if BOT_USERNAME in text:
            new_text: str = text.replace(BOT_USERNAME, '').strip()
            response: str = handle_response(new_text)
        else:
            return
    else:
        # Check if the message contains an emoji
        if "✔" in update.message.text or "✅" in update.message.text or "☑" in update.message.text:
            # Handle the ✔ emoji reaction
            if update.message.reply_to_message:
                replied_message = update.message.reply_to_message
                original_message_text = replied_message.text
                if(is_valid_message(original_message_text)):
                    handle_approval(original_message_text)
                    response = "You reacted with ✔ - success"
            else: response = "please reply to a specific message"
        elif "❌" in update.message.text or "❎" in update.message.text:
            # Handle the ❌ emoji reaction
            if update.message.reply_to_message:
                replied_message = update.message.reply_to_message
                original_message_text = replied_message.text
                if is_valid_message(original_message_text):
                    handle_decline(original_message_text)
                    response = "You reacted with ❌"
            else: response = "please reply to a specific message"
        else: response: str = handle_response(text)

    print('bot:', response)
    await update.message.reply_text(response)

#errors
async def error(update: Update, context: ContextTypes.DEFAULT_TYPE):
    print(f'Update {update} caused error {context.error}')

if __name__ == '__main__':
    print('starting bot...')
    app = Application.builder().token(TOKEN).build()

    #commands
    app.add_handler(CommandHandler('start', start_command))
    app.add_handler(CommandHandler('check_firestore', check_firestore))
    app.add_handler(CommandHandler('showApprovedUsers', showApprovedUsers))

    #messages
    app.add_handler(MessageHandler(filters.TEXT, handle_message))

    #errors
    app.add_error_handler(error)

    #polls the bot
    print('polling...')
    app.run_polling(poll_interval=3)

