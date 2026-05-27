import os
os.environ['TF_ENABLE_ONEDNN_OPTS'] = '0'
import numpy as np
import math
import matplotlib.pyplot as plt
import tensorflow as tf
from tensorflow.keras import layers
import tf2onnx
import onnx

# Function: Convert some hex value into an array for C programming
def hex_to_c_array(hex_data, var_name):
    c_str = ''

    # Create header guard
    c_str += '#ifndef ' + var_name.upper() + '_H\n'
    c_str += '#define ' + var_name.upper() + '_H\n\n'

    # Add array length at top of file
    c_str += '\nunsigned int ' + var_name + '_len = ' + str(len(hex_data)) + ';\n'

    # Declare C variable
    c_str += 'unsigned char ' + var_name + '[] = {'
    hex_array = []
    for i, val in enumerate(hex_data):

        # Construct string from hex
        hex_str = format(val, '#04x')

        # Add formatting so each line stays within 80 characters
        if (i + 1) < len(hex_data):
            hex_str += ','
        if (i + 1) % 12 == 0:
            hex_str += '\n '
        hex_array.append(hex_str)

    # Add closing brace
    c_str += '\n ' + format(' '.join(hex_array)) + '\n};\n\n'

    # Close out header guard
    c_str += '#endif //' + var_name.upper() + '_H'

    return c_str

def main():
    nsamples = 1000
    val_ratio = 0.2
    test_ratio = 0.2

    # Generate some random samples
    np.random.seed(1234)
    x_values = np.random.uniform(low=0, high=(2 * math.pi), size=nsamples)

    # Create a noisy sinewave with these values
    y_values = np.sin(x_values) + (0.1 * np.random.randn(x_values.shape[0]))

    shuffle_idx = np.random.permutation(nsamples)
    x_values = x_values[shuffle_idx]
    y_values = y_values[shuffle_idx]

    # Split the dataset into training, validation and test.
    val_split = int(val_ratio * nsamples)
    test_split = int(val_split + (test_ratio * nsamples))
    x_val, x_test, x_train = np.split(x_values, [val_split, test_split])
    y_val, y_test, y_train = np.split(y_values, [val_split, test_split])

    # Check that our splits add up correctly
    assert(x_train.size + x_val.size + x_test.size) == nsamples

    # Plot the data in each partition in different colors:
    plt.plot(x_train, y_train, 'b.', label="Train")
    plt.plot(x_test, y_test, 'r.', label="Test")
    plt.plot(x_val, y_val, 'y.', label="Validate")
    plt.legend()
    plt.show()

    # Create a model
    model = tf.keras.Sequential([
        tf.keras.Input(shape=(1,)),
        layers.Dense(48, activation='relu'),
        layers.Dense(32, activation='relu'),
        layers.Dense(32, activation='relu'),
        layers.Dense(16, activation='relu'),
        layers.Dense(1)
    ])

    model.compile(optimizer='adam', loss='mae', metrics=['mae'])

    history = model.fit(x_train,
                        y_train,
                        epochs=500,
                        batch_size=16,
                        validation_data=(x_val, y_val))

    # Plot the training history
    loss = history.history['loss']
    val_loss = history.history['val_loss']
    epochs = range(1, len(loss) + 1)

    plt.plot(epochs, loss, 'bo', label='Training loss')
    plt.plot(epochs, val_loss, 'b', label='Validation loss')
    plt.title('Training and validation loss')
    plt.legend()
    plt.show()

    sort_idx = np.argsort(x_test)
    x_test_sorted = x_test[sort_idx]
    y_test_sorted = y_test[sort_idx]
    predictions = model.predict(x_test_sorted)

    plt.figure(figsize=(10, 6))
    plt.plot(x_test_sorted, y_test_sorted, 'b.', label='Actual')
    plt.plot(x_test_sorted, predictions, 'r.', label='Prediction')
    plt.xlabel(r'$\pi$ (Radians)')
    plt.ylabel(r'Amplitude ($\sin(x)$)')
    plt.title('Predicted compared to actual Sine Wave')
    plt.legend()
    plt.show()

    # Calculate accuracy of predictions
    x_all_sorted = np.sort(x_values)
    predictions_all = model.predict(x_all_sorted)
    actual_all = np.sin(x_all_sorted)

    # Mean Absolute Error
    mae = np.mean(np.abs(predictions_all.flatten() - actual_all))

    # Mean Squared Error
    mse = np.mean((predictions_all.flatten() - actual_all) ** 2)

    # Root Mean Squared Error
    rmse = np.sqrt(mse)

    # R-squared score
    ss_res = np.sum((actual_all - predictions_all.flatten()) ** 2)
    ss_tot = np.sum((actual_all - np.mean(actual_all)) ** 2)
    r_squared = 1 - (ss_res / ss_tot)

    print(f"Mean Absolute Error (MAE):  {mae:.4f}")
    print(f"Mean Squared Error (MSE):   {mse:.4f}")
    print(f"Root Mean Squared Error:    {rmse:.4f}")
    print(f"R-squared Score:            {r_squared:.4f}")

    total_params = model.count_params()
    print(f"Total trainable parameters: {total_params}")

    # Save as .keras format
    model.save('sine_model.keras')
    print("Model saved as sine_model.keras")

    # Save .keras
    model.save('sine_model.keras')
    print("Model saved as sine_model.keras")

    # Convert to TensorFlow Lite
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    tflite_model = converter.convert()
    with open('sine_model.tflite', 'wb') as f:
        f.write(tflite_model)
    print("Model saved as sine_model.tflite")

    with open('sine_model.tflite', 'rb') as f:
        data = f.read()

    c_model_name = 'sine_model'

    # Write TFLite model to a C source (or header) file
    with open(c_model_name + '.h', 'w') as file:
        file.write(hex_to_c_array(tflite_model, c_model_name ))
if __name__ == "__main__":
    main()