function Checkout({ cart, onClose, onPlaceOrder }) {
  const total = cart.reduce(
    (sum, item) => sum + item.price * item.quantity,
    0
  )

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

      <div className="bg-white rounded-2xl shadow-xl w-full max-w-2xl p-8 max-h-[90vh] overflow-y-auto">

        <div className="flex justify-between items-center mb-6">
          <h2 className="text-3xl font-bold text-gray-800">
            Checkout
          </h2>

          <button
            onClick={onClose}
            className="text-gray-500 text-2xl hover:text-gray-800"
          >
            ×
          </button>
        </div>

        {/* Address */}
        <h3 className="text-xl font-bold text-gray-800 mb-4">
          Delivery Address
        </h3>

        <textarea
          placeholder="Enter your complete delivery address"
          rows="3"
          className="w-full border rounded-lg px-4 py-3 mb-6"
        />

        {/* Payment */}
        <h3 className="text-xl font-bold text-gray-800 mb-4">
          Payment Method
        </h3>

        <div className="grid grid-cols-2 gap-3 mb-6">

          {["Cash on Delivery", "Card", "UPI", "Wallet"].map(
            (method) => (
              <label
                key={method}
                className="border rounded-lg p-4 cursor-pointer
                           hover:border-orange-500"
              >
                <input
                  type="radio"
                  name="payment"
                  value={method}
                  defaultChecked={method === "Cash on Delivery"}
                  className="mr-2"
                />

                {method}
              </label>
            )
          )}

        </div>

        {/* Order Summary */}
        <h3 className="text-xl font-bold text-gray-800 mb-4">
          Order Summary
        </h3>

        <div className="space-y-3">

          {cart.map((item) => (
            <div
              key={item.id}
              className="flex justify-between border-b pb-2"
            >
              <span>
                {item.name} × {item.quantity}
              </span>

              <span className="font-semibold">
                ₹{item.price * item.quantity}
              </span>
            </div>
          ))}

        </div>

        <div className="flex justify-between text-2xl font-bold mt-6">
          <span>Total</span>

          <span className="text-orange-600">
            ₹{total}
          </span>
        </div>

        <button
          onClick={onPlaceOrder}
          className="w-full bg-orange-600 text-white py-4 rounded-lg
                     font-bold mt-6 hover:bg-orange-700"
        >
          Place Order
        </button>

      </div>

    </div>
  )
}

export default Checkout