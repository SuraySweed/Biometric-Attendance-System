from typing import final
import re
import logging
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
    await update.message.reply_text('welcome to BAS_FB_bot!')

async def info_command(update: Update, context: ContextTypes.DEFAULT_TYPE):
    bot_info_message = """
*BAS\_FB\_bot Info*

This bot allows you to interact with a Firestore database and perform various tasks\.

*Available Commands:*
\- /start: Start the bot and get a welcome message\.
\- /info: show bot info and commands\.
\- /check\_firestore: Check pending users in the Firestore database\.
\- /approved\_users: Show approved users in the Firestore database\.
\- /rejected\_users: Show rejected users in the Firestore database\.
\- /reject: Reject a user from the pending users list \(replace `ID` with the user's FingerID\)\.
\- /accept: Accept a user and move them to the approved users list \(replace `ID` with the user's FingerID\)\.

*Usage:*
To reject a user: `/reject \<ID\>` or `/reject \<all\>`
To accept a user: `/accept \<ID\>` or `/accept \<all\>`

Enjoy using the bot 🙂 \!
    """
    await update.message.reply_text(bot_info_message, parse_mode='MarkdownV2')

async def check_firestore(update: Update, context: CallbackContext):
    try:
        # Read data from Firebase
        db = firestore.client()
        collection_name = 'PendingUsers'

        # Check if the collection exists and is not empty
        collection_ref = db.collection(collection_name)
        documents = collection_ref.stream()
        if len(list(documents)) == 0:
            await update.message.reply_text("The collection is empty or does not exist.")
            return

        # Log the total number of documents in the collection
        logging.info(f"Total documents in '{collection_name}': {len(list(documents))}")
        # Reset the documents generator to iterate over them again
        documents = collection_ref.stream()
        usersList_message = "pending users:\n"
        for doc in documents:
            # Log document ID before processing
            logging.info(f"Processing document: {doc.id}")

            # Get data from the current document
            document_data = doc.to_dict()
            if document_data:
                if(document_data.get('data', {}).get('isPending')):
                    fingerID = document_data.get('data', {}).get('FingerPrintID')
                    userID = document_data.get('data', {}).get('id')

                    if fingerID is not None and userID is not None:
                        #logging.info(f"finger print id is: {fingerID} and user id is: {userID} registered at: {time}")
                        usersList_message += f"user{fingerID} ID: {userID}\n"
                    else:
                        logging.warning(f"Field not found or is empty in document: {doc.id}")
            else:
                logging.warning(f"Document data is empty for document: {doc.id}")
        await update.message.reply_text(usersList_message)
        # Log a message after processing all documents
        logging.info("Finished processing documents")

    except Exception as e:
        # Handle exceptions here, log the error, and potentially send an error message to the user
        logging.error(f"Error processing Firestore documents: {str(e)}")
        await update.message.reply_text("An error occurred while processing Firestore documents.")

async def showApprovedUsers(update: Update, context: CallbackContext):
    try:
        # Read data from Firebase
        db = firestore.client()
        collection_name = 'ApprovedUsers'

        # Check if the collection exists and is not empty
        collection_ref = db.collection(collection_name)
        documents = collection_ref.stream()
        if len(list(documents)) == 0:
            await update.message.reply_text("The collection is empty or does not exist.")
            return
        # Log the total number of documents in the collection
        logging.info(f"Total documents in '{collection_name}': {len(list(documents))}")

        # Reset the documents generator to iterate over them again
        documents = collection_ref.stream()
        usersList_message = "approved users:\n"
        for doc in documents:
            # Log document ID before processing
            logging.info(f"Processing document: {doc.id}")

            # Get data from the current document
            document_data = doc.to_dict()

            if document_data:
                fingerID = document_data.get('data', {}).get('FingerPrintID')
                userID = document_data.get('data', {}).get('id')
                time = document_data.get('Time')

                if fingerID is not None and userID is not None:
                    usersList_message += f"user{fingerID} ID: {userID}, logged in at {time}\n"
                else:
                    logging.warning(f"Field not found or is empty in document: {doc.id}")
            else:
                logging.warning(f"Document data is empty for document: {doc.id}")
        await update.message.reply_text(usersList_message)
        # Log a message after processing all documents
        logging.info("Finished processing documents")

    except Exception as e:
        # Handle exceptions here, log the error, and potentially send an error message to the user
        logging.error(f"Error processing Firestore documents: {str(e)}")
        await update.message.reply_text("An error occurred while processing Firestore documents.")

async def showRejectedUsers(update: Update, context: CallbackContext):
    try:
        # Read data from Firebase
        db = firestore.client()
        collection_name = 'RejectedUsers'

        # Check if the collection exists and is not empty
        collection_ref = db.collection(collection_name)
        documents = collection_ref.stream()
        if len(list(documents)) == 0:
            await update.message.reply_text("The collection is empty or does not exist.")
            return
        # Log the total number of documents in the collection
        logging.info(f"Total documents in '{collection_name}': {len(list(documents))}")

        # Reset the documents generator to iterate over them again
        documents = collection_ref.stream()
        usersList_message = "rejected users:\n"
        for doc in documents:
            # Log document ID before processing
            logging.info(f"Processing document: {doc.id}")

            # Get data from the current document
            document_data = doc.to_dict()

            if document_data:
                fingerID = document_data.get('data', {}).get('FingerPrintID')
                userID = document_data.get('data', {}).get('id')
                time = document_data.get('Time')

                if fingerID is not None and userID is not None:
                    usersList_message += f"user{fingerID} ID: {userID}, logged in at {time}\n"
                else:
                    logging.warning(f"Field not found or is empty in document: {doc.id}")
            else:
                logging.warning(f"Document data is empty for document: {doc.id}")
        await update.message.reply_text(usersList_message)
        # Log a message after processing all documents
        logging.info("Finished processing documents")

    except Exception as e:
        # Handle exceptions here, log the error, and potentially send an error message to the user
        logging.error(f"Error processing Firestore documents: {str(e)}")
        await update.message.reply_text("An error occurred while processing Firestore documents.")

async def accept(update: Update, context: CallbackContext):
    if len(context.args) == 1:
        try:
            db1 = firestore.client()
            pendingList = db1.collection('PendingUsers')
            db2 = firestore.client()
            approvedUsers = db2.collection('ApprovedUsers')
            updated = False
            if(context.args[0] == 'all'):
                documents = pendingList.stream()
                for doc in documents:
                    doc_toAdd = doc.to_dict()
                    if(doc_toAdd.get('data', {}).get('isPending')):
                        id_number = doc_toAdd.get('data', {}).get('FingerPrintID')
                        doc_toAdd['data']['isPending'] = False  # update the field isPending in pendingUsers
                        document_ref = pendingList.document(f'user{id_number}')
                        document_ref.set(doc_toAdd)
                        doc_toAdd_name = f"user{id_number}"
                        approvedUsers.document(doc_toAdd_name).set(doc_toAdd)
                        updated = True
                if updated:
                    print("all user have been add to ApprovedUsers collection\n")
                    result_message = "all users have been accepted"
                else:
                    print("no users to add\n")
                    result_message = "no uesrs to add"
            else:
                #get the user document from pending list
                id_number = int(context.args[0])
                document = f'user{id_number}'
                document_ref = pendingList.document(document)
                doc = pendingList.document(document).get()
                #add user to approved users list
                if doc.exists:
                    doc_toAdd = doc.to_dict()
                    if (doc_toAdd.get('data', {}).get('isPending')):
                        doc_toAdd_name = f"user{id_number}"
                        doc_toAdd['data']['isPending'] = False #update the field isPending in pendingUsers
                        document_ref.set(doc_toAdd)
                        updated = True
                        approvedUsers.document(doc_toAdd_name).set(doc_toAdd)
                if updated:
                    print(f"user{id_number} has been add to approved collection")
                    result_message = f"added user with finger ID {id_number}"
                else:
                    print(f"can't accept user{id_number}")
                    result_message = f"can't accept user{id_number}"

        except ValueError:
            result_message = "Invalid ID number. Please provide a valid integer."
    else:
        result_message = "Usage: /accept <ID>/all"
    await update.message.reply_text(result_message)


async def reject(update: Update, context: CallbackContext):
    if len(context.args) == 1:
        try:
            db1 = firestore.client()
            pendingList = db1.collection('PendingUsers')
            db2 = firestore.client()
            RejectedUsers = db2.collection('RejectedUsers')
            updated = False
            #reject all users
            if(context.args[0] == 'all'):
                documents = pendingList.stream()
                for doc in documents:
                    doc_toAdd = doc.to_dict()
                    if (doc_toAdd.get('data', {}).get('isPending')):
                        id_number = doc_toAdd.get('data', {}).get('FingerPrintID')
                        doc_toAdd['data']['isPending'] = False  # update the field isPending in pendingUsers
                        document_ref = pendingList.document(f'user{id_number}')
                        document_ref.set(doc_toAdd)
                        doc_toAdd_name = f"user{id_number}"
                        RejectedUsers.document(doc_toAdd_name).set(doc_toAdd)
                        updated = True
                if updated:
                    print("all user have been add to rejected collection\n")
                    result_message = "all users have been rejected"
                else:
                    print("no users to reject\n")
                    result_message = "no uesrs to reject"
                #await update.message.reply_text(result_message)
            else:
                #get user document from PendingUsers collection
                id_number = int(context.args[0])
                document = f'user{id_number}'
                document_ref = pendingList.document(document)
                doc = pendingList.document(document).get()
                #document_ref.delete()
                print(f"user{id_number} has been rejected")
                #result_message = f"rejectd user with finger ID {id_number}"

                #add user document to RejectedUsers collection
                if doc.exists:
                    doc_toAdd = doc.to_dict()
                    if (doc_toAdd.get('data', {}).get('isPending')):
                        doc_toAdd_name = f"user{id_number}"
                        doc_toAdd['data']['isPending'] = False  # update the field isPending in pendingUsers
                        document_ref.set(doc_toAdd)
                        RejectedUsers.document(doc_toAdd_name).set(doc_toAdd)
                        updated = True
                if updated:
                    print(f"user{id_number} has been add to rejected collection")
                    result_message = f"rejected user with finger ID {id_number}"
                else:
                    print(f"can't reject user{id_number}")
                    result_message = f"can't reject user{id_number}"
        except ValueError:
            result_message = "Invalid ID number. Please provide a valid integer."
    else:
        result_message = "Usage: /reject <ID>/all"
    await update.message.reply_text(result_message)

#Responses
def handle_response(text: str) -> str:
    processed: str = text.lower()
    if 'hello' in processed:
        return 'hey there'
    elif 'how are you' in processed:
        return 'I am good, Thank you!'
    else: return 'BAKAAAAA'

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
        response: str = handle_response(text)

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
    app.add_handler(CommandHandler('info', info_command))
    app.add_handler(CommandHandler('check_firestore', check_firestore))
    app.add_handler(CommandHandler('approved_users', showApprovedUsers))
    app.add_handler(CommandHandler('rejected_users', showRejectedUsers))
    app.add_handler(CommandHandler("reject", reject))
    app.add_handler(CommandHandler("accept", accept))

    #messages
    app.add_handler(MessageHandler(filters.TEXT, handle_message))

    #errors
    app.add_error_handler(error)

    #polls the bot
    print('polling...')
    app.run_polling(poll_interval=3)

